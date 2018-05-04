/*
  Wick contractor - Operator
  Author: Christoph Lehner
  Date: 2018
*/
class Operator {
public:
  std::vector<OperatorTerm> t;

  void dump() {
    std::cout << "[" << t.size() << "]{" << std::endl;
    for (auto& i : t)
      i.dump();
    std::cout << "}" << std::endl;
  }

  void write(FILE* f) {
    for (auto& i : t) {
      i.write(f);
      fprintf(f,"\n");
    }
  }

  Operator() {
  }

  template<typename F>
  void apply_bilinear(F f) {
    for (auto& i : t) {
      for (auto& b : i.qbi) {
        f(b);
      }
    }
  }

  void simplify() {
    for (int i=(int)t.size()-1;i>=0;i--) {
      int j;
      for (j=0;j<i;j++) {
        if (match(t[i],t[j]))
          break;
      }
      if (j != i) {
        t[j].factor += t[i].factor;
        t.erase(t.begin() + i);
      }
    }

    // now remove all zeros
    for (int i=(int)t.size()-1;i>=0;i--)
      if (norm(t[i].factor) < 1e-28)
        t.erase(t.begin() + i);
  }

  Operator(FileParser& p) {

    try {
      while (!p.eof())
        t.push_back( OperatorTerm( p ) );
    } catch (ParserSpeculationFail pe) {
    }

    if (!p.eof()) {
      p.dump();
      assert(0);
    }

  }

};
