all: wick simplify hc tr cseop replace

wick: src/main.cc
	mpiicpc -o wick -O3 -std=c++11 src/main.cc

replace: src/replace.cc
	mpiicpc -o replace -O3 -std=c++11 src/replace.cc

simplify: src/simplify.cc
	mpiicpc -o simplify -O3 -std=c++11 src/simplify.cc

cseop: src/cseop.cc
	mpiicpc -o cseop -O3 -std=c++11 src/cseop.cc

hc: src/hc.cc
	mpiicpc -o hc -O3 -std=c++11 src/hc.cc

tr: src/tr.cc
	mpiicpc -o tr -O3 -std=c++11 src/tr.cc
