/*
  Wick contractor - OperatorTerm
  Author: Christoph Lehner
  Date: 2018
*/
class OperatorTerm {
public:
  Complex factor;
  std::vector<QuarkBilinear> qbi;
  std::map< std::string, std::vector<Complex> > hints;

  void dump() const {
    std::cout << " (" << factor << ") * [" << std::endl;
    for (auto& i : qbi)
      i.dump();
    std::cout << " ]" << std::endl;
  }

  void write(FILE* f) {
    fprintf(f,"FACTOR %.15g %.15g\n",factor.real(),factor.imag());
    for (auto& i : qbi)
      i.write(f);
  }

  void parse_hint(std::string h) {
    auto a = split(h,'[');
    assert(a.size()==2);
    auto a2 = split(a[1],',');
    std::vector<Complex> v;
    for (size_t i=0;i<a2.size();i++)
      v.push_back(atof(a2[i].c_str()));
    hints[a[0]] = v;
  }

  OperatorTerm operator*(const OperatorTerm& b) const {
    OperatorTerm res;
    res.factor = factor * b.factor;
    res.qbi.insert(std::end(res.qbi), std::begin(qbi), std::end(qbi));
    res.qbi.insert(std::end(res.qbi), std::begin(b.qbi), std::end(b.qbi));
    return res;
  }

  OperatorTerm() : factor(1.0) {
  }

  OperatorTerm(FileParser& p) {

    if (!p.is("FACTOR"))
      throw ParserSpeculationFail();

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
    } catch (ParserSpeculationFail pe) {
    }

    while (!p.eof() && p.is("HINT:")) {
      auto& a = p.get();
      for (size_t i=1;i<a.size();i++)
        parse_hint(a[i]);
    }
  }
};
