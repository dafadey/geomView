#include "tools.h"

std::string remove_chars(const std::string& input, char c) {
  std::string out;
  for (size_t i = 0; i < input.size(); i++) {
    if (input.c_str()[i] == c)
      continue;
    out += input.c_str()[i];
  }
  return out;
}

std::vector<std::string> split(const std::string& input, const std::string& delimiter) {
  size_t pos = 0;
  std::vector<std::string> out;
  while (true) {
    size_t end = input.find(delimiter, pos);
    out.push_back(input.substr(pos, end - pos));
    pos = end + delimiter.size();
    if (end == std::string::npos)
      break;
  }
  return out;
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
      v="";
      continue;
    }
    v += c;
  }
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