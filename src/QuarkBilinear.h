/*
  Wick contractor - Quark Bilinear
  Author: Christoph Lehner
  Date: 2018
*/
class QuarkBilinear {
public:

  std::vector< std::vector< std::string > > lines;

  void dump() const {
    for (auto& i : lines) {
      for (auto& j : i)
        std::cout << j << ",";
      std::cout << std::endl;
    }
  }

  void write(FILE* f) {
    for (auto& i : lines) {
      fprintf(f,"%s",i[0].c_str());
      for (int j=1;j<i.size();j++)
        fprintf(f," %s",i[j].c_str());
      fprintf(f,"\n");
    }
  }

  QuarkBilinear() { }

  QuarkBilinear(FileParser& p) {
    if (p.is("UBAR") || p.is("DBAR")) {
      do {
        lines.push_back(p.get());
      } while (!p.was("U") && !p.was("D"));
    } else if (p.is("BEGINTRACE")) {
      do {
        lines.push_back(p.get());
      } while (!p.was("ENDTRACE"));
    } else {
      throw ParserSpeculationFail();
    }
  }
};
