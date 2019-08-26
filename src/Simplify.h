/*
  Wick contractor - Simplify
  Author: Christoph Lehner
  Date: 2018
*/
int compare(const std::vector<std::string>& ln, const std::vector<std::string>& ln2) {
  if (ln.size() != ln2.size())
    return -1;
  for (int i=0;i<ln.size();i++)
    if (ln[i].compare(ln2[i]) != 0)
      return 1;
  return 0;
}

template<typename T>
int compare(const T& a, const T& b, int i0, int n0, int off) {
  for (int i=0;i<n0;i++) {
    int j = (i + off) % n0;
    if (compare(a[i+i0],b[j+i0]) != 0)
      return 1;
  }
  return 0;
}

bool match(const QuarkBilinear& a, const QuarkBilinear& b) {
  // if a trace try all cyclic versions, if not a trace strings must match
  if (a.lines.size() != b.lines.size())
    return false;

  assert(a.lines.size());

  if (a.lines[0][0].compare("BEGINTRACE") == 0 && compare(a.lines[0],b.lines[0]) == 0) {
    // do a traces compare
    for (int off=0;off<a.lines.size()-2;off++) {
      if (compare(a.lines,b.lines,1,a.lines.size()-2,off)==0)
        return true;
    }
  } else {

    if (compare(a.lines,b.lines,0,a.lines.size(),0)==0)
      return true;
  }

  return false;
}

bool match(const OperatorTerm& a, const OperatorTerm& b) {
  if (a.qbi.size() != b.qbi.size())
    return false;

  std::vector<QuarkBilinear> tomatch_a = a.qbi;
  std::vector<QuarkBilinear> tomatch_b = b.qbi;
  while (tomatch_a.size()) {
    auto& ta = tomatch_a[0];
    int i;
    for (i=0;i<tomatch_b.size();i++)
      if (match(ta,tomatch_b[i]))
        break;
    if (i != tomatch_b.size()) {
      tomatch_b.erase(tomatch_b.begin()+i);
      tomatch_a.erase(tomatch_a.begin());
    } else {
      return false;
    }
  }
  return true;
}

std::string join(const std::vector<std::string>& vec, const char* delim) {
  std::stringstream res;
  std::copy(vec.begin(), vec.end(), std::ostream_iterator<std::string>(res, delim));
  return res.str();
}

std::string get_hash(const OperatorTerm& t) {
  std::vector<std::string> sz;
  for (const auto & i : t.qbi) {
    for (const auto & l : i.lines) {
      sz.push_back(join(l," "));
    }
  }
  sort(sz.begin(),sz.end());
  return join(sz,"|");
}

std::string get_pair_key(const QuarkBilinear& qbi, int i) {
  int N = (int)qbi.lines.size()-2;
  assert(i < N);
  int ip = (i + 1) % N;
  assert(i != ip); // should never happen / just checks if N==1
  return join(qbi.lines[i+1]," ") + "|" + join(qbi.lines[ip+1]," ");
}

void bilinear_from_pair(QuarkBilinear& qbi, std::string key) {
  auto a = split(key,'|');
  for (auto x : a) {
    auto b = split(x,' ');
    std::vector<std::string> r;
    for (auto y : b) {
      if (y.size())
	r.push_back(y);
    }
    qbi.lines.push_back(r);
  }
}

void replace_pair(QuarkBilinear& qbi, std::string key, std::string tag) {

  // only work on traces
  if (qbi.lines[0][0].compare("BEGINTRACE"))
    return;

  int N=(int)qbi.lines.size()-2;
  for (int i=0;i<N;i++) {
    if (N<2)
      break;
    if (!get_pair_key(qbi,i).compare(key)) {
      qbi.lines.erase(qbi.lines.begin() + i + 1);
      if (i==N - 1)
	qbi.lines[0+1] = { "EVALM", tag };
      else
	qbi.lines[i+1] = { "EVALM", tag };
      i--;
      N--;
    }
  }
}

void add_pair_counts(std::map<std::string,int>& counts, const QuarkBilinear& qbi) {

  // only work on traces
  if (qbi.lines[0][0].compare("BEGINTRACE"))
    return;

  int N=(int)qbi.lines.size()-2;
  if (N<2)
    return;

  for (int i=0;i<N;i++) {
    std::string k = get_pair_key(qbi,i);
    auto j = counts.find(k);
    if (j==counts.end())
      counts[k]=1;
    else
      counts[k]+=1;
  }
}

bool cse_step(std::map<std::string,QuarkBilinear>& defs, int min_occ, int& idx) {

  std::map<std::string,int> counts;
  for (const auto& c : defs)
    add_pair_counts(counts,c.second);

  // find max
  std::string kmax;
  int nmax = -1;
  for (const auto& c : counts)
    if (c.second > nmax) {
      nmax = c.second;
      kmax = c.first;
    }

  if (!mpi_id) {
    std::cout << "# " << nmax << " occurrences of " << kmax << std::endl;
  }

  if (nmax < min_occ)
    return false;

  // create new
  char tag[256];
  sprintf(tag,"B%9.9d",idx++);
  QuarkBilinear qbi;
  bilinear_from_pair(qbi,kmax);

  // replace all
  for (auto& c : defs)
    replace_pair(c.second,kmax,tag);

  // add new
  defs[tag] = qbi;
  return true;
}

void cse_steps(std::map<std::string,QuarkBilinear>& defs, int min_occ) {
  int idx = 0;
  while (cse_step(defs,min_occ,idx));
}


