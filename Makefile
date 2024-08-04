all: wick simplify hc tr cseop replace

wick: src/main.cc
	mpicxx -o wick -O3 -std=c++11 src/main.cc

replace: src/replace.cc
	mpicxx -o replace -O3 -std=c++11 src/replace.cc

simplify: src/simplify.cc
	mpicxx -o simplify -O3 -std=c++11 src/simplify.cc

cseop: src/cseop.cc
	mpicxx -o cseop -O3 -std=c++11 src/cseop.cc

hc: src/hc.cc
	mpicxx -o hc -O3 -std=c++11 src/hc.cc

tr: src/tr.cc
	mpicxx -o tr -O3 -std=c++11 src/tr.cc
