#include "tools.h"

std::string SFW(const std::wstring& in) {
	char tmp[333];
	std::wcstombs(tmp, in.c_str(), 333);
	return std::string(tmp);
}

std::vector<std::string> split_vectors(const std::string& input) {
  std::vector<std::string> output;
  std::string v;
  char const* it = input.c_str();
  for(int i=0;i<input.size();i++,it++) {
    char c = *it;
    if(c=='(' || c==')' ) {
      if(!v.empty())
        output.push_back(v);
      v.clear();
      continue;
    }
    v += c;
  }
  if(!v.empty())
    output.push_back(v);
  return output;
}

vec3f make_vector(const std::string& input) {
  vec3f output;
  std::string v;
  char const* it = input.c_str();
  int coord = 0;
  for (int i = 0; i < input.size(); i++, it++) {
    char c = *it;
    if (c == ',') {
      if(coord < 3)
        output[coord] = atof(v.c_str());
      coord++;
      v = "";
      continue;
    }
    v += c;
  }
  if (coord < 3)
    output[coord] = atof(v.c_str());
  return output;
}
