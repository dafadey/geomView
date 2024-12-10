#pragma once
#include <Windows.h>
#include <winbase.h>
#include <fileapi.h>
#include <string>
#include <vector>
#include "tools.h"
#define MAXSTRING 333

namespace FSNAMESPACE {
namespace filesystem {
  
  static std::string SFW(const std::wstring& in) {
    char tmp[MAXSTRING];
    std::wcstombs(tmp, in.c_str(), MAXSTRING);
    return std::string(tmp);
  }


  const std::wstring separator = L"\\";

  struct path : public std::vector<std::wstring> {
    path() = default;

    std::wstring wstring() const {
      std::wstring out;
      if (!this->size())
        return out;
      for (int i = 0; i < this->size() - 1; i++)
        out += (*this)[i] + separator;
      out += this->back();
      return out;
    }

    path(const std::wstring& s) : std::vector<std::wstring>(split(s, separator)) {}

    path filename() const {
      if (this->size())
        return path(this->back());
      else
        return path();
    }
  };

  struct dirEntry : public std::wstring {
    dirEntry(const std::wstring& s) : std::wstring(s) {}
    path path() const { return path::path(*this); }
  };

  struct directory_iterator : public std::vector<dirEntry> {
    directory_iterator(path p) {
      HANDLE hFind = INVALID_HANDLE_VALUE;
      auto pws = p.wstring();
      dirEntry currentDir(pws+L"\\*");
      WIN32_FIND_DATA ffd;
      hFind = FindFirstFile(currentDir.c_str(), &ffd);
      if (INVALID_HANDLE_VALUE == hFind)
        return;
      const auto& name = std::wstring(ffd.cFileName);
      if (name != L"." && name != L"..")
        push_back(pws + name);
      while (FindNextFile(hFind, &ffd) != 0) {
        const auto& name = std::wstring(ffd.cFileName);
        if (name != L"." && name != L"..")
          push_back(pws + name);
      }
      FindClose(hFind);
    }
  };

  bool is_directory(const dirEntry& de) {
    return ::GetFileAttributes(de.c_str()) & FILE_ATTRIBUTE_DIRECTORY;
  }

  path current_path() {
    wchar_t cp[MAXSTRING];
    size_t n = ::GetCurrentDirectory(MAXSTRING, cp);
    if (n < MAXSTRING)
      return path(std::wstring(cp)+separator);
    else
      return path(std::wstring() + separator);
  }
}//namespace
}//namespace
#undef MAXSTRING
