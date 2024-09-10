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
    FileParser p(a[i+2].c_str());
    ret += scale * Operator(p);
  }
  return ret;
}

bool isint(std::string s) {
  std::string::const_iterator it = s.begin();
  while (it != s.end() && std::isdigit(*it)) ++it;
  return !s.empty() && it == s.end();
}

int commute_gammas(std::vector< std::vector<std::string> >& lines, Complex& fac, int rho) {
  int n = 0;
  for (int i=0;i<lines.size()-1;i++) {
    if (strcmp(lines[i][0].c_str(),"GAMMA")==0 &&
	strcmp(lines[i+1][0].c_str(),"GAMMA")==0) {
      assert(lines[i].size() == 2);
      assert(lines[i+1].size() == 2);
      
      if (isint(lines[i][1]) && isint(lines[i+1][1])) {
	
	int mu = atoi(lines[i][1].c_str());
	int nu = atoi(lines[i+1][1].c_str());
	
	assert(mu >= 0 && mu <=5 && mu != 4);
	assert(nu >= 0 && nu <=5 && nu != 4);
	
	if (mu == rho) {
	  
	  if (mu == nu) {
	    // all gammas squared are 1
	    lines.erase(lines.begin()+i+1);
	    lines.erase(lines.begin()+i);
	    i=0;
	    n++;
	  } else {
	    
	    std::vector<std::string> tmp = lines[i];
	    lines[i] = lines[i+1];
	    lines[i+1] = tmp;
	    fac *= -1;
	    n++;
	    
	  }
	}
      }
    }
  }
  return n;
}

int move_gammas_to_front(std::vector< std::vector<std::string> >& lines, Complex& fac) {
  int n = 0;
  for (int i=0;i<lines.size()-1;i++) {
    if (strcmp(lines[i+1][0].c_str(),"GAMMA")==0 &&
	(strcmp(lines[i][0].c_str(),"MOM")==0 ||
	 strcmp(lines[i][0].c_str(),"MOMDAG")==0)) {

      std::vector<std::string> tmp = lines[i];
      lines[i] = lines[i+1];
      lines[i+1] = tmp;

      n++;
    }
  }

  return n;
}

void combine_gammas(std::vector< std::vector<std::string> >& lines, Complex& fac) {
  // first move all gammas to front
  while(move_gammas_to_front(lines,fac));

  // then order gammas
  for (int rho=0;rho<=5;rho++) {
    while(commute_gammas(lines,fac,rho));
  }
}

QuarkBilinear cc(QuarkBilinear in, Complex& fac) {
  QuarkBilinear out;
  // ubar gamma d -> (-1)^2 u^T gamma^T dbar^T = (-1)^3 (dbar gamma u)^T = -dbar gamma u
  fac *= -1; // final operator permutation
  for (auto l : in.lines) {


    const char* c = l[0].c_str();
    if (strcmp(c,"D") == 0 || strcmp(c,"U") == 0 || strcmp(c,"S") == 0) {
      l[0] = l[0] + "BAR";
      out.lines.insert(out.lines.begin(),l);
    } else if (strcmp(c,"DBAR") == 0 || strcmp(c,"UBAR") == 0 || strcmp(c,"SBAR") == 0) {
      l[0] = l[0].substr(0,1);
      out.lines.insert(out.lines.begin(),l);
      fac *= -1;
    } else if (strcmp(c,"MOM") == 0) {
      // CC: U -> U^* => S(x,y) -> S(x,y)^*
      // psibar(x) S(x,y) e^{iyp} S(y,z) psi(z) -> -psi(x)^T S(x,y)* e^{iyp} S(y,z)* psibar(z)^T
      // = psibar(z) S(z,y) e^{iyp} S(y,x) psi(x)
      // S(x,y)* = <x|n>* <n|y>* = <y|n><n|x> = S(y,x)
      out.lines.insert(out.lines.begin(),l);
    } else if (strcmp(c,"GAMMA") == 0) {
      fac *= -1;
      out.lines.insert(out.lines.begin(),l); // all the gammas are hermitian
    } else if (strcmp(c,"MOMDAG") == 0) {
      out.lines.insert(out.lines.begin(),l);
    } else {
      std::cout<<"Unknown " << l[0] << std::endl;
      assert(0);
    }

  }

  combine_gammas(out.lines,fac);

  return out;
}

OperatorTerm cc(OperatorTerm in) {
  OperatorTerm out;
  out.hints = in.hints;
  out.factor = in.factor;
  for (auto& qbi : in.qbi) {
    out.qbi.push_back(cc(qbi,out.factor));
  }
  return out;
}

Operator cc(Operator in) {
  Operator out;
  for (auto& t : in.t) {
    out.t.push_back(cc(t));
  }
  return out;
}

int main(int argc, char* argv[]) {
  if (argc < 2)
    return 1;

  MPI_Init(&argc,&argv);
  MPI_Comm_size (MPI_COMM_WORLD,&mpi_n);
  MPI_Comm_rank (MPI_COMM_WORLD, &mpi_id);

  cc(load(argv[1])).write(stdout);
  
  MPI_Finalize();
  return 0;
}
