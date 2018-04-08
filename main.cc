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

typedef std::complex<double> Complex;

template<typename Out>
void split(const std::string &s, char delim, Out result) {
  std::stringstream ss;
  ss.str(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    *(result++) = item;
  }
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}

std::string trim(const std::string& sc) {
  std::string s = sc;
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                  std::not1(std::ptr_fun<int, int>(std::isspace))));
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

class FileParser {
public:
  std::vector< std::vector<std::string> > cmds;
  size_t pos;

  FileParser(const char* fn) : pos(0) {
    FILE* f = fopen(fn,"rt");
    assert(f);
    char line[2048];
    while (!feof(f)) {
      if (!fgets(line,sizeof(line),f))
        break;
      auto tline = trim(line);
      if (tline[0]=='#')
        continue;
      auto cmd = split(tline,' ');
      if (!cmd.empty())
        cmds.push_back(cmd);      
    }
    fclose(f);
  }

  bool eof() {
    return (pos == cmds.size());
  }

  bool is(const char* tag) {
    assert(!eof());
    return cmds[pos][0].compare(tag) == 0;
  }

  bool was(const char* tag) {
    assert(pos > 0);
    return cmds[pos-1][0].compare(tag) == 0;
  }

  std::vector<std::string>& get() {
    assert(!eof());
    return cmds[pos++];
  }

  void next() {
    assert(!eof());
    pos++;
  }

  void convert(std::string s, std::string& r) {
    r = s;
  }

  void convert(std::string s, double& d) {
    d = atof(s.c_str());
  }

  template<typename T>
  T get(size_t i) {
    assert(!eof());
    assert(i < cmds[pos].size());
    T r;
    convert(cmds[pos][i],r);    
    return r;
  }

  size_t nargs() {
    assert(!eof());
    return cmds[pos].size() - 1;
  }

  void dump() {
    std::cout << "FileParser {" << std::endl;
    for (size_t i=0;i<cmds.size();i++) {
      auto c = cmds[i];
      if (i == pos)
        std::cout << "--> ";
      std::cout << "(";
      for (auto& a : c) {
        std::cout << a << ",";
      }
      std::cout << ")" << std::endl;
    }
    std::cout << "}" << std::endl;
  }
};

class ParserError {
};

class QuarkBilinear {
public:

  std::vector< std::vector< std::string > > lines;

  QuarkBilinear(FileParser& p) {
    if (p.is("UBAR") || p.is("DBAR")) {
      do {
        lines.push_back(p.get());
      } while (!p.was("U") && !p.was("D"));
    } else {
      throw ParserError();
    }
  }
};

class OperatorTerm {
public:
  Complex factor;
  std::vector<QuarkBilinear> qbi;
  std::vector< std::vector<std::string> > hints;

  OperatorTerm(FileParser& p) {

    if (!p.is("FACTOR"))
      throw ParserError();

    // get factor
    if (p.nargs()==2) {
      factor = Complex(p.get<double>(1),p.get<double>(2));
    } else {
      factor = p.get<double>(1);
    }
    p.next();

    try {
      while (!p.eof())
        qbi.push_back( QuarkBilinear( p ) );
    } catch (ParserError pe) {
    }

    while (!p.eof() && p.is("HINT:")) {
      hints.push_back(p.get());
    }
  }
};

class Operator {
public:
  std::vector<OperatorTerm> t;

  Operator(FileParser& p) {

    try {
      while (!p.eof())
        t.push_back( OperatorTerm( p ) );
    } catch (ParserError pe) {
    }

    if (!p.eof()) {
      p.dump();
      assert(0);
    }

  }

};

int main(int argc, char* argv[]) {
  if (argc < 3)
    return 1;
  
  FileParser p1(argv[1]);
  FileParser p2(argv[2]);

  Operator op1(p1);
  Operator op2(p2);

  return 0;
}
