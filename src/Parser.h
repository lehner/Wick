/*
  Wick contractor - Parser
  Author: Christoph Lehner
  Date: 2018
*/
template<typename Out>
void split(const std::string &s, char delim, Out result) {
  std::stringstream ss;
  ss.str(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    *(result++) = item;
  }
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}

std::string trim(const std::string& sc) {
  std::string s = sc;
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                  std::not1(std::ptr_fun<int, int>(std::isspace))));
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

class FileParser {
public:
  std::vector< std::vector<std::string> > cmds;
  size_t pos;

  void replace(const char* a, const char* b) {
    for (auto& i : cmds) {
      for (auto& j : i) {
	if (!strncmp(j.c_str(),"EVAL",4)) {
	  fprintf(stderr,"Cannot use replace if EVAL* is used!\n");
	  exit(3);
	}
        if (!strcmp(a,j.c_str()))
          j=b;
      }
      i.erase(std::remove_if(i.begin(),i.end(),
                             [](std::string const& s) { return s.size() == 0; }),
              i.end());
    }
  }

  FileParser(const char* fn) : pos(0) {
    FILE* f = fopen(fn,"rt");
    if (!f)
      fprintf(stderr,"Could not open file %s\n",fn);
    assert(f);
    char line[2048];
    while (!feof(f)) {
      if (!fgets(line,sizeof(line),f))
        break;
      auto tline = trim(line);
      if (tline[0]=='#')
        continue;
      auto cmd = split(tline,' ');
      if (!cmd.empty())
        cmds.push_back(cmd);      
    }
    fclose(f);

#ifdef MPI_VERSION
    if (!mpi_id)
      std::cout << "# Parsed " << fn << ": " << cmds.size() << " commands" << std::endl;
#endif
  }

  bool eof() {
    return (pos == cmds.size());
  }

  bool is(const char* tag) {
    assert(!eof());
    return cmds[pos][0].compare(tag) == 0;
  }

  bool was(const char* tag) {
    assert(pos > 0);
    return cmds[pos-1][0].compare(tag) == 0;
  }

  std::vector<std::string>& get() {
    assert(!eof());
    return cmds[pos++];
  }

  void next() {
    assert(!eof());
    pos++;
  }

  void convert(std::string s, std::string& r) {
    r = s;
  }

  void convert(std::string s, double& d) {
    d = atof(s.c_str());
  }

  template<typename T>
  T get(size_t i) {
    assert(!eof());
    assert(i < cmds[pos].size());
    T r;
    convert(cmds[pos][i],r);    
    return r;
  }

  size_t nargs() {
    assert(!eof());
    return cmds[pos].size() - 1;
  }

  void dump() const {
    std::cout << "FileParser {" << std::endl;
    for (size_t i=0;i<cmds.size();i++) {
      auto c = cmds[i];
      if (i == pos)
        std::cout << "--> ";
      std::cout << "(";
      for (auto& a : c) {
        std::cout << a << ",";
      }
      std::cout << ")" << std::endl;
    }
    std::cout << "}" << std::endl;
  }
};

class ParserSpeculationFail {
};
