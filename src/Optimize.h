/*
  Wick contractor - Optimize
  Author: Christoph Lehner
  Date: 2018
*/
bool is_prop_type(std::string& s) {
  for (auto& fm : flavor_map) {
    if (fm.second.compare(s) == 0)
      return true;
  }
  return false;
}

void replace_source(QuarkBilinear& qbi, char* tsrc) {
  for (auto & l : qbi.lines) {
    if (is_prop_type(l[0])) {
      if (l.size() == 3) {
        if (l[2].compare(tsrc) == 0) {
          l[0] = l[0] + "BAR";
        }
      }
    }
  }
}

bool is_prop_sl(std::vector<std::string>& c, std::string& t0, std::string& t1) {
  if (c.size() != 4)
    return false;
  if (!is_prop_type(c[0]))
    return false;
  if (c[3].compare("local"))
    return false;
  t0 = c[1];
  t1 = c[2];
  return true;
}

bool is_prop_ls(std::vector<std::string>& c, std::string& t0, std::string& t1) {
  if (c.size() != 4)
    return false;
  if (!is_prop_type(c[0]))
    return false;
  if (c[2].compare("local"))
    return false;
  t0 = c[1];
  t1 = c[3];
  return true;
}

bool is_prop_ll(std::vector<std::string>& c, std::string& t0, std::string& t1) {
  if (c.size() != 5)
    return false;
  if (!is_prop_type(c[0]))
    return false;
  if (c[2].compare("local"))
    return false;
  if (c[4].compare("local"))
    return false;
  t0 = c[1];
  t1 = c[3];
  return true;
}

bool is_gamma(std::vector<std::string>& c, std::string& mu) {
  if (c.size() != 2)
    return false;
  if (c[0].compare("GAMMA"))
    return false;
  mu = c[1];
  return true;
}

void replace_combined_operators(QuarkBilinear& qbi) {
  std::vector< std::vector< std::string > > lines;
  std::string t0,t1,t2,t3,mu;

  std::vector< std::string > hash;
  hash.push_back("#");

  if (qbi.lines[0][0].compare("BEGINTRACE") == 0) {
    
    int n = qbi.lines.size()-2;
    for (int i=0;i<n;i++) {
      
      int ip = (i + 1) % n;
      int ipp = (i + 2) % n;

      if (is_prop_sl(qbi.lines[1+i],t0,t1) &&
          is_gamma(qbi.lines[1+ip],mu) &&
          is_prop_ls(qbi.lines[1+ipp],t2,t3) &&
          t1.compare(t2) == 0) {

        std::vector< std::string > op;
        op.push_back(qbi.lines[1+i][0] + "_LGAMMA_" + qbi.lines[1+ipp][0]);
        op.push_back(t0);
        op.push_back(t1);
        op.push_back(mu);
        op.push_back(t3);

        qbi.lines[1+i] = op;
        qbi.lines[1+ip] = hash;
        qbi.lines[1+ipp] = hash;
      }

      if (is_prop_ll(qbi.lines[1+i],t0,t1) &&
	  t0.compare(t1) == 0 &&
	  is_gamma(qbi.lines[1+ip],mu) && n == 2) {
        std::vector< std::string > op;
        op.push_back(qbi.lines[1+i][0] + "_LTADPOLE");
        op.push_back(t0);
        op.push_back(mu);

        qbi.lines[0] = op;
        qbi.lines[1] = hash;
	qbi.lines[2] = hash;
	qbi.lines[3] = hash;
      }

    }

  }

  qbi.lines.erase(std::remove_if(qbi.lines.begin(),qbi.lines.end(),
                                 [](std::vector<std::string> const& s) { return s[0].compare("#")==0; }),
          qbi.lines.end());

}
