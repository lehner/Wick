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

  std::map<std::string,QuarkBilinear> cse(const std::string& prefix = "") {

    std::map<std::string,QuarkBilinear> ret;
    int idx = 0;
    char tag[1024];
    int repeat = 0, unique = 0;
    for (auto& i : t) {
      for (auto & qb : i.qbi) {
	if (qb.lines.size() && qb.lines[0].size() && !qb.lines[0][0].compare("BEGINTRACE")) {
	  bool has = false;
	  for (auto & ex : ret) {
	    if (match(ex.second,qb)) {
	      has = true;
	      qb.lines.clear();
	      qb.lines.push_back({ "EVAL", ex.first });
	      repeat++;
	      break;
	    }
	  }
	  if (!has) {
	    sprintf(tag,"CS%5.5d",idx++);
	    std::string st = prefix + tag;
	    ret[st] = qb;
	    qb.lines.clear();
	    qb.lines.push_back({ "EVAL", st });
	    unique++;
	  }
	}
      }
    }

    if (!mpi_id) {
      std::cout << "# cse found " << unique << " unique out of " << (unique + repeat) << " total traces" << std::endl;
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
      if (!mpi_id)
	printf("# group %d / %d with %d elements simplified to %d elements\n",i,(int)grps.size(),
	       n0,n);
      for (auto & ti : grps[i].t)
	t.push_back(ti);
    }
  }

  void simplify() {

    std::vector<int> n_rank_matches(mpi_n);
    int i = (int)t.size() - 1;
    while (i > 0) {
      // logic: for each match group, let only element with lowest index survive

      // first step: get all matches for element i
      std::vector<int> lmatches;
      int j;
      int i_per_rank = i / mpi_n;
      if (i_per_rank*mpi_n < i)
	i_per_rank++;
      assert(i_per_rank*mpi_n >= i);
      for (j=i_per_rank*mpi_id;j<i_per_rank*(mpi_id+1);j++) { // don't break ordering!!
	if (j>=i)
	  break;
        if (match(t[i],t[j]))
	  lmatches.push_back(j);
      }
      // gather matches
      int nrank = (int)lmatches.size();
      MPI_Allgather(&nrank,1,MPI_INT,&n_rank_matches[0],1,MPI_INT,MPI_COMM_WORLD);
      int n = 0;
      for (j=0;j<mpi_n;j++)
	n += n_rank_matches[j];
      std::vector<int> _matches(n,0), matches(n,0);
      n = 0;
      for (j=0;j<mpi_id;j++)
	n += n_rank_matches[j];
      for (j=0;j<(int)lmatches.size();j++)
	_matches[n+j]=lmatches[j];
      MPI_Allreduce(&_matches[0],&matches[0],(int)matches.size(),MPI_INT,MPI_SUM,MPI_COMM_WORLD);
      matches.push_back(i);

      if (matches.size() > 1) {
	int survivor = matches[0];
	for (j=(int)matches.size()-1;j>0;j--) {
	  int dup = matches[j];
	  t[survivor].factor += t[dup].factor;
	  t.erase(t.begin() + dup);
	}
	i++;
      }

      i -= (int)matches.size();
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
