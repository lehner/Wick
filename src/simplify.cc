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

std::map< char, std::string > flavor_map = { {'U',"LIGHT"}, {'D',"LIGHT"}, {'S',"STRANGE"} };

typedef std::complex<double> Complex;

#include "Parser.h"
#include "QuarkBilinear.h"
#include "OperatorTerm.h"
#include "Simplify.h"
#include "Operator.h"
#include "Wick.h"
#include "Optimize.h"

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

int main(int argc, char* argv[]) {
  if (argc < 2)
    return 1;

  FileParser p1(argv[1]);
  Operator op1(p1);  

  size_t norig = op1.t.size();
  op1.simplify();
  size_t nsimpl = op1.t.size();

  std::cout << "# Simplified " << norig << " to " << nsimpl << " terms" << std::endl;
  op1.write(stdout);
  return 0;
}
