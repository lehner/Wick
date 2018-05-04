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
  if (argc < 3)
    return 1;

  // write input as output
  printf("#");
  for (int i=0;i<argc;i++)
    printf(" %s",argv[i]);
  printf("\n#\n");

  int nops;
  if (!strncmp(argv[2],"--",2)) {
    nops = 1;
  } else {
    nops = 2;
  }

  Operator res;
  int s_a(0), s_t(0);
    
  if (nops == 2) {
    FileParser p1(argv[1]);
    FileParser p2(argv[2]);
    
    // get additional parameters
    for (int i=nops+1;i<argc;i++) {
      if (!strcmp(argv[i],"--replace_right")) {
	assert(i+2 < argc);
	p2.replace(argv[i+1],argv[i+2]);
      } else if (!strcmp(argv[i],"--replace_left")) {
	assert(i+2 < argc);
	p1.replace(argv[i+1],argv[i+2]);
      }
    }
    
    Operator op1(p1);
    Operator op2(p2);
    
    // now go through all combinations of operator terms and see if their hints match
    // if so, perform wick contractions for them
    for (auto& ot1 : op1.t) {
      for (auto& ot2 : op2.t) {
	if (hints_match(ot1.hints,ot2.hints)) {
	  add_wick(res,ot1*ot2); s_a++;
	}
	s_t++;
      }
    }
  } else if (nops == 1) {
    FileParser p1(argv[1]);
    
    // get additional parameters
    for (int i=nops+1;i<argc;i++) {
      if (!strcmp(argv[i],"--replace_left")) {
	assert(i+2 < argc);
	p1.replace(argv[i+1],argv[i+2]);
      }
    }
    
    Operator op1(p1);
    for (auto& ot1 : op1.t) {
      add_wick(res,ot1); 
      s_a++;
      s_t++;
    }
  } else {
    printf("# Nops = %d not yet implemented!\n",nops);
    return 2;
  }
    
  // simplify result
  printf("#\n");
  printf("# %d / %d combinations have matching hints\n",s_a,s_t);
  printf("# %d term(s) before simplification\n",(int)res.t.size());
  res.simplify();
  printf("# %d term(s) after simplification\n",(int)res.t.size());
  printf("#\n");

  // get additional parameters
  for (int i=nops+1;i<argc;i++) {
    if (!strcmp(argv[i],"--avoid_source")) {
      assert(i+1 < argc);
      res.apply_bilinear([argv,i](QuarkBilinear& b) { replace_source(b,argv[i+1]); });
    }
  }

  // identify combined operators
  res.apply_bilinear(replace_combined_operators);

  printf("\n");
  res.write(stdout);

  return 0;
}
