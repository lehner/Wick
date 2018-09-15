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
