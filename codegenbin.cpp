#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
	if (argc < 2) {
		return -1;
	}
  for(int arg =1; arg<argc; arg++)
  {
    std::string name = argv[arg];
    std::ifstream inf(name.c_str(), std::ios_base::binary);
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inf), {});
    inf.close();

    std::string varname;
    for (int i = 0; i < name.size(); i++) {
      char c = name.c_str()[i];
      if (c == '.')
        c = '_';
      varname += c;
      if (c == '\\' || c == '/')
        varname.clear();
    }
    std::ios_base::fmtflags f(std::cout.flags());
    std::cout << "const char " << varname << "[] = {";
    for (int i = 0; i < buffer.size(); i++) {
      if (i % 16 == 0)
        std::cout << '\n';
      std::cout << " (char) 0x" << std::setw(2) << std::setfill('0') << std::hex << (int) buffer[i] << ',';
      }
    std::cout << "\n};\n";
    std::cout.flags(f);
    std::cout << "const unsigned int " << varname << "_len = " << buffer.size() << ";\n\n";
  }
	return 0;
}
