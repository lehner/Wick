/*
  Replace terms
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

int main(int argc, char* argv[]) {
  if (argc < 3)
    return 1;

  MPI_Init(&argc,&argv);
  MPI_Comm_size (MPI_COMM_WORLD,&mpi_n);
  MPI_Comm_rank (MPI_COMM_WORLD, &mpi_id);

  if (!mpi_id) {
    // write input as output
    printf("#");
    for (int i=0;i<argc;i++)
      printf(" %s",argv[i]);
    printf("\n#\n");
    printf("# MPI with %d ranks\n",mpi_n);
    printf("#\n");
  }

  int nops;
  for (nops=1;nops<argc;nops++) {
    if (!strncmp(argv[nops],"--",2))
      break;
  }
  nops--;

  assert(nops==1);

  {
    FileParser p1(argv[1]);
    
    // get additional parameters
    for (int i=nops+1;i<argc;i++) {
      if (!strcmp(argv[i],"--replace_left")) {
	assert(i+2 < argc);
	p1.replace(argv[i+1],argv[i+2]);
      }
    }
    
    Operator op1(p1);
    op1.write(stdout);
  }

  MPI_Finalize();
  return 0;
}
