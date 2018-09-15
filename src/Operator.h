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

  std::vector<Operator> split_by_hash() {
    std::map< std::string, std::vector<int> > groups;
    for (int i=0;i<(int)t.size();i++)
      groups[get_hash(t[i])].push_back(i);
    std::vector<Operator> ret;
    for (auto & group : groups) {
      Operator o;
      for (auto & i : group.second)
	o.t.push_back(t[i]);
      ret.push_back(o);
    }
    return ret;
  }

  void simplify_with_heuristics() {
    // first perform O(N) operation to find potential matches
    std::vector<Operator> grps = split_by_hash();
    t.clear();
    for (int i=0;i<(int)grps.size();i++) {
      int n0 = (int)grps[i].t.size();
      grps[i].simplify();
      int n = (int)grps[i].t.size();
      printf("# group %d / %d with %d elements simplified to %d elements\n",i,(int)grps.size(),
	     n0,n);
      for (auto & ti : grps[i].t)
	t.push_back(ti);
    }
  }

  void simplify() {
    // strategy: first find list of matches which can be done in parallel
    // then act on them
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
