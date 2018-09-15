all: wick simplify wick_mpi

wick: src/main.cc
	icpc -o wick -O3 -std=c++11 src/main.cc

wick_mpi: src/main_mpi.cc
	mpiicpc -o wick_mpi -qopenmp -O3 -std=c++11 src/main_mpi.cc

simplify: src/simplify.cc
	icpc -o simplify -O3 -std=c++11 src/simplify.cc
