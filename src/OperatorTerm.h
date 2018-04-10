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

  void parse_hint(std::vector<std::string> h) {
    assert(h.size() > 2);
    std::string name = h[1];
    std::vector<Complex> v;
    for (size_t i=2;i<h.size();i++) {
      auto a = split(h[i],',');
      if (a.size() == 1) {
        v.push_back(atof(a[0].c_str()));
      } else if (a.size() == 2) {
        v.push_back(Complex(atof(a[0].c_str()),atof(a[1].c_str())));
      } else {
        assert(0);
      }
    }

    hints[name] = v;
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

    while (!p.eof() && p.is("HINT")) {
      parse_hint(p.get());
    }
  }
};
