/*
  Wick contractor - Slow wick contractor
  Author: Christoph Lehner
  Date: 2018
*/
bool is_bilinear(QuarkBilinear& qbi, char& fbar, char& f) {
  size_t n = qbi.lines.size();
  assert(n);
  const char* a = qbi.lines[0][0].c_str();
  const char* e = qbi.lines[n-1][0].c_str();
  if (!strcmp(a+1,"BAR")) {
    fbar = a[0];
    f = e[0];
    return true;
  }
  return false;
}

int get_first_bilinear(OperatorTerm& t, char& fbar, char& f) {
  for (int i=0;i<(int)t.qbi.size();i++) {
    if (is_bilinear(t.qbi[i],fbar,f))
      return i;
  }
  return -1;
}

std::string get_prop_type(char t) {
  auto f = flavor_map.find(t);
  assert(f != flavor_map.end());
  return f->second;    
}

QuarkBilinear merge(const QuarkBilinear& a, const QuarkBilinear& b, char type) {
  QuarkBilinear r;
  int i;
  std::vector< std::string > prop;
  auto& pa = a.lines[a.lines.size()-1];
  auto& pb = b.lines[0];
  prop.push_back(get_prop_type(type));
  for (i=1;i<pa.size();i++)
    prop.push_back(pa[i]);
  for (i=1;i<pb.size();i++)
    prop.push_back(pb[i]);

  for (i=0;i<a.lines.size()-1;i++)
    r.lines.push_back(a.lines[i]);
  r.lines.push_back(prop);
  for (i=1;i<b.lines.size();i++)
    r.lines.push_back(b.lines[i]);
  return r;
}

QuarkBilinear merge(const QuarkBilinear& a, char type) {
  QuarkBilinear r;
  int i;
  std::vector< std::string > prop;
  auto& pa = a.lines[a.lines.size()-1];
  auto& pb = a.lines[0];
  prop.push_back(get_prop_type(type));
  for (i=1;i<pa.size();i++)
    prop.push_back(pa[i]);
  for (i=1;i<pb.size();i++)
    prop.push_back(pb[i]);

  r.lines.push_back({ "BEGINTRACE" });
  for (i=1;i<a.lines.size()-1;i++)
    r.lines.push_back(a.lines[i]);
  r.lines.push_back(prop);
  r.lines.push_back({ "ENDTRACE" });
  return r;
}

void add_wick(Operator& r, OperatorTerm t) {

  // get first remaining quark bilinear
  char fbar, f;
  int i = get_first_bilinear(t,fbar,f);
  if (i == -1) {
    r.t.push_back(t);
    return;
  }

  // contract it with all other remaining bilinears, each contraction done in a new target that we then add together in the end
  for (int j=0;j<(int)t.qbi.size();j++) {
    char _fbar, _f;
    if (is_bilinear(t.qbi[j],_fbar,_f) && f == _fbar) {
      // merge the two to a single term with a propagator
      OperatorTerm tm;
      tm.factor = t.factor;
      if (i==j)
        tm.factor *= -1.0;
      int l;
      for (l=0;l<(int)t.qbi.size();l++)
        if (l != i && l != j)
          tm.qbi.push_back(t.qbi[l]);
      if (i==j)
        tm.qbi.push_back(merge(t.qbi[i],f));
      else
        tm.qbi.push_back(merge(t.qbi[i],t.qbi[j],f));
      add_wick(r,tm);
    }
  }
}
