#pragma once

#include <vector>
#include <string>

#include "vectors.h"

std::vector<std::string> split_vectors(const std::string& input);

vec3f make_vector(const std::string& input);


template<typename Tstring, typename Tchar>
Tstring remove_chars(const Tstring& input, Tchar c) {
  Tstring out;
  for (size_t i = 0; i < input.size(); i++) {
    if (input.c_str()[i] == c)
      continue;
    out += input.c_str()[i];
  }
  return out;
}

template<typename Tstring>
std::vector<Tstring> split(const Tstring& input, const Tstring& delimiter) {
  size_t pos = 0;
  std::vector<Tstring> out;
  while (true) {
    size_t end = input.find(delimiter, pos);
    out.push_back(input.substr(pos, end - pos));
    pos = end + delimiter.size();
    if (end == std::string::npos)
      break;
  }
  return out;
}

template std::string remove_chars<std::string, char>(const std::string&, char);
template std::wstring remove_chars<std::wstring, wchar_t>(const std::wstring&, wchar_t);
template std::vector<std::string> split<std::string>(const std::string&, const std::string&);
template std::vector<std::wstring> split<std::wstring>(const std::wstring&, const std::wstring&);
