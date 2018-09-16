all: wick simplify

wick: src/main.cc
	mpiicpc -o wick -O3 -std=c++11 src/main.cc

simplify: src/simplify.cc
	icpc -o simplify -O3 -std=c++11 src/simplify.cc
