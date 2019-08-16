/*
  Wick contractor
  Author: Christoph Lehner
  Date: 2018
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

Operator load(char* cmd) {
  if (cmd[0]!='@') {
    FileParser p(cmd);
    return Operator(p);
  }
  auto a = split(std::string(&cmd[1]),'@');
  assert(a.size()%3==0);

  Operator ret;
  for (int i=0;i<(int)a.size();i+=3) {
    Complex scale = Complex(atof(a[i+0].c_str()),atof(a[i+1].c_str()));

    auto pf = split(a[i+2],'|');
    assert(pf.size() > 0);
    FileParser p0(pf[0].c_str());
    Operator op = Operator(p0);
    for (int j=1;j<(int)pf.size();j++) {
      FileParser pj(pf[j].c_str());
      op = op*Operator(pj);
    }
    ret += scale * op;
  }
  return ret;
}
int main(int argc, char* argv[]) {
  if (argc < 2)
    return 1;

  MPI_Init(&argc,&argv);
  MPI_Comm_size (MPI_COMM_WORLD,&mpi_n);
  MPI_Comm_rank (MPI_COMM_WORLD, &mpi_id);

  bool do_cse = true;
  int nops, i;
  for (nops=1;nops<argc;nops++) {
    if (!strncmp(argv[nops],"--",2))
      break;
  }
  nops--;
  for (i=nops+1;i<argc;i++) {
    if (!strcmp(argv[i],"--nocse")) {
      do_cse=false;
    } else {
      fprintf(stderr,"Unknown command %s\n",argv[i]);
      return 1;
    }
      
  }

  std::vector<Operator> ops;
  for (i=0;i<nops;i++) {
    ops.push_back(load(argv[1+i]));
  }

  for (i=0;i<nops;i++) {
    size_t norig = ops[i].t.size();
    ops[i].simplify();
    size_t nsimpl = ops[i].t.size();
  
    if (!mpi_id) {
      std::cout << "# Simplified operator " << argv[1+i] << " from " << norig << " to " << nsimpl << " terms" << std::endl;
    }
  }

  std::map<std::string,QuarkBilinear> defs;
  if (do_cse) {
    cse(defs,ops);
    cse_steps(defs,2);
    if (!mpi_id) {
      std::cout << "# CSE defines " << defs.size() << " terms" << std::endl;
    }
  }

  if (!mpi_id) {
    for (auto& d : defs) {
      if (!d.second.lines[0][0].compare("BEGINTRACE")) {
	printf("\nBEGINDEFINE\n");
	d.second.write(stdout);
	printf("ENDDEFINE %s\n\n",d.first.c_str());
      } else {
	printf("\nBEGINMDEFINE\n");
	d.second.write(stdout);
	printf("ENDMDEFINE %s\n\n",d.first.c_str());
      }
    }
  
    if (nops == 1) {
      ops[0].write(stdout);
    } else {
      for (i=0;i<nops;i++) {
	std::string fno = std::string(argv[1+i]) + ".simplified";
	if (exists(fno)) {
	  fprintf(stderr,"Error: %s already exists!\n", fno.c_str());
	  exit(1);
	}
	FILE* f = fopen(fno.c_str(),"wt");
	assert(f);
	fprintf(f,"# Original: %s\n",argv[1+i]);
	ops[i].write(f);
	fclose(f);
      }
    }
  }
  
  MPI_Finalize();
  return 0;
}
