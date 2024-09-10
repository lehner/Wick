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


QuarkBilinear isospin(QuarkBilinear in, Complex& fac) {
  for (auto& l : in.lines) {

    const char* c = l[0].c_str();
    if (strcmp(c,"D") == 0) {
      l[0] = "U";
    } else if (strcmp(c,"U") == 0) {
      l[0] = "D";
      fac *= -1;
    } else if (strcmp(c,"DBAR") == 0) {
      l[0] = "UBAR";
    } else if (strcmp(c,"UBAR") == 0) {
      l[0] = "DBAR";
      fac *= -1;
    }


  }

  return in;
}

OperatorTerm isospin(OperatorTerm in) {
  OperatorTerm out;
  out.hints = in.hints;
  out.factor = in.factor;
  for (auto& qbi : in.qbi) {
    out.qbi.push_back(isospin(qbi,out.factor));
  }
  return out;
}

Operator isospin(Operator in) {
  Operator out;
  for (auto& t : in.t) {
    out.t.push_back(isospin(t));
  }
  return out;
}

int main(int argc, char* argv[]) {
  if (argc < 2)
    return 1;

  MPI_Init(&argc,&argv);
  MPI_Comm_size (MPI_COMM_WORLD,&mpi_n);
  MPI_Comm_rank (MPI_COMM_WORLD, &mpi_id);

  isospin(load(argv[1])).write(stdout);
  
  MPI_Finalize();
  return 0;
}
