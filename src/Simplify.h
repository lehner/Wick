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

  if (a.lines[0][0].compare("BEGIN_TRACE") == 0 && compare(a.lines[0],b.lines[0]) == 0) {
    // do a traces compare
    for (int off=0;off<a.lines.size()-2;off++) {
      if (compare(a.lines,b.lines,1,a.lines.size()-2,off)==0)
        return true;
    }
  } else {
    std::cout << "Match:" << std::endl;
    a.dump();
    b.dump();
    assert(0); // right now only handle full traces
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
