/*
  Wick contractor
  Author: Christoph Lehner
  Date: 2019
*/
#include <stdio.h>
#include <vector>
#include <iostream>
#include <complex>
#include <assert.h>
#include <iterator>
#include <sstream>
#include <map>
#include <algorithm>
#include <string.h>
#include <mpi.h>
#include <sys/stat.h>

int mpi_id, mpi_n;

std::map< char, std::string > flavor_map = { {'U',"LIGHT"}, {'D',"LIGHT"}, {'S',"STRANGE"} };

typedef std::complex<double> Complex;

#include "Parser.h"
#include "QuarkBilinear.h"
#include "OperatorTerm.h"
#include "Simplify.h"
#include "Operator.h"
#include "Wick.h"
#include "Optimize.h"

bool exists(const std::string& file) {
  struct stat buf;
  return (stat(file.c_str(), &buf) == 0);
}

Complex sp(std::vector<Complex>& a, std::vector<Complex>& b) {
  assert(a.size() == b.size());
  Complex r;
  for (size_t i=0;i<a.size();i++)
    r+=conj(a[i])*b[i];
  return r;
}

template<typename T>
bool hints_match(T& a, T& b) {
  // first check that all keys are in both
  for (auto& h1 : a)
    if (b.find(h1.first) == b.end())
      return false;
  for (auto& h2 : b)
    if (a.find(h2.first) == a.end())
      return false;
  // then compute scalar products
  for (auto& h1 : a) {
    Complex s = sp(h1.second,b[h1.first]);
    if (norm(s) == 0.0)
      return false;
    //std::cout << s << std::endl;
  }
  return true;
}

// checks if a==aOverb * b
bool isratio(Operator& a, Operator& b, Complex& aOverb) {
  // for now force same order
  if (a.t.size() == b.t.size()) {
    if (a.t.size() > 0) {
      Complex expAoverB;
      if (b.t[0].qbi.size() != 1 ||
	  a.t[0].qbi.size() != 1)
	return false;

      if (match(a.t[0].qbi[0],b.t[0].qbi[0])) {
	expAoverB = a.t[0].factor / b.t[0].factor;
	for (size_t i = 1;i<a.t.size();i++) {

	  if (b.t[i].qbi.size() != 1 ||
	      a.t[i].qbi.size() != 1)
	    return false;

	  if (!match(a.t[i].qbi[0],b.t[i].qbi[0]))
	    return false;

	  Complex AoverB = a.t[i].factor / b.t[i].factor;
	  double eps = abs((expAoverB - AoverB) / expAoverB);
	  if (eps > 1e-12)
	    return false;
	}
	aOverb = expAoverB;
	return true;
      }
    }
  }
  return false;
}

bool remove_redundant_defs(std::map<std::string,Operator>& defs,
			   Operator& o, char* buf) {

  for (auto& d1 : defs) {
    if (d1.first.substr(0,strlen(buf)).compare(buf)==0) {

      Complex rd1Overd2;
      for (auto& d2 : defs) {
	if (d2.first.compare(d1.first)) {
	 
	  if (isratio(d1.second,d2.second,rd1Overd2)) {
	    // d1 = rd1Overd2 d2, replace d1 in o
	    for (auto& t : o.t){
	      for (auto& q : t.qbi) {
		for (auto& l : q.lines) {
		  if (l.size() == 2 && l[0].compare("EVALM")==0 &&
		      l[1].compare(d1.first)==0) {
		    l[1] = d2.first;
		    t.factor *= rd1Overd2;
		  }
		}
	      }
	    }
	    
	    // remove d1.first
	    defs.erase(d1.first);
	    return true;
	  }
	}
      }
    }
  }

  return false;
}

void bilinear_cse(Operator& o,
		  std::map<std::string,Operator>& defs,
		  int& nr) {

  char buf[64];
  sprintf(buf,"%8.8d",nr++);

  Operator res;
  for (auto& t : o.t) {
    if (t.qbi.size() != 1) {
      res.t.push_back(t);
    } else {

      // only combine operator terms with simple bilinears

      auto& qb = t.qbi[0];
      if (qb.lines.size()<2) {
	res.t.push_back(t);
      } else {

	assert(qb.lines[0].size());
	assert(qb.lines[qb.lines.size()-1].size());
	std::string lf = qb.lines[0][0];
	std::string ll = qb.lines[qb.lines.size()-1][0];
	if ( (lf.compare("UBAR")==0 || lf.compare("DBAR")==0 ||
	      lf.compare("SBAR")==0) &&
	     (ll.compare("U")==0 || ll.compare("D")==0 ||
	      ll.compare("S")==0) ) {

	  Operator op0;
	  OperatorTerm tp0;
	  QuarkBilinear qb0;
	  tp0.factor = t.factor;
	  for (size_t j=1;j<qb.lines.size()-1;j++)
	    qb0.lines.push_back(qb.lines[j]);
	  tp0.qbi.push_back(qb0);
	  op0.t.push_back(tp0);

	  std::string tag = std::string(buf) + "@" + join(qb.lines[0],"@") + "@" + join(qb.lines[qb.lines.size()-1],"@");
	  auto d = defs.find(tag);
	  if (d==defs.end()) {
	    // change qb + add to def
	    OperatorTerm tp;
	    QuarkBilinear qb1;
	    qb1.lines.push_back(qb.lines[0]);
	    qb1.lines.push_back({ "EVALM", tag });
	    qb1.lines.push_back(qb.lines[qb.lines.size()-1]);
	    tp.factor = 1.0;
	    tp.qbi.push_back(qb1);
	    res.t.push_back(tp);

	    defs[tag] = op0;
	  } else {
	    defs[tag] += op0;
	  }
	} else {
	  res.t.push_back(t);
	}

      }
    }
  }

  // check for duplicates for buf-buf and buf-*, no need for *-*
  while(remove_redundant_defs(defs,res,buf));
  o=res;
}

int main(int argc, char* argv[]) {
  if (argc < 2)
    return 1;

  MPI_Init(&argc,&argv);
  MPI_Comm_size (MPI_COMM_WORLD,&mpi_n);
  MPI_Comm_rank (MPI_COMM_WORLD, &mpi_id);

  int nops = argc - 1, i,nr=0;
  std::vector<Operator> ops;
  std::map<std::string,Operator> defs;

  for (i=0;i<nops;i++) {
    FileParser p(argv[1+i]);
    Operator o(p);
  
    bilinear_cse(o,defs,nr);

    std::string fno = std::string(argv[1+i]) + ".simplified";
    if (exists(fno)) {
      fprintf(stderr,"Error: %s already exists!\n", fno.c_str());
      exit(1);
    }

    FILE* f = fopen(fno.c_str(),"wt");
    assert(f);
    fprintf(f,"# Original: %s\n",argv[1+i]);
    o.write(f);
    fclose(f);
  }

  if (!mpi_id) {
    for (auto& d : defs) {
      int n=0;
      for (auto& t : d.second.t) {
	printf("\nBEGINMDEFINE %.15g %.15g\n",t.factor.real(),t.factor.imag());
	assert(t.qbi.size() == 1);
	t.qbi[0].write(stdout);
	printf("ENDMDEFINE %s%s\n\n",d.first.c_str(),n == 0 ? "" : " +"); // TODO add +
	n++;
      }
    }
  }
  
  MPI_Finalize();
  return 0;
}
