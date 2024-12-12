#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include "geom_view.h"

std::vector<std::string> parse(const std::string& s) {
  std::vector<std::string> res;
  std::string buf;
  for(int i = 0; s.c_str()[i] != 0; i++) {
    if(s.c_str()[i]==' ') {
      res.push_back(buf);
      buf.clear();
    } else
      buf += s.c_str()[i];
  }
  if(buf.size())
    res.push_back(buf);
  return res;
}

int main() {

  geom_view gv;
  gv.init();

  std::string cmd="";
  while(cmd != "exit") {
    getline(std::cin, cmd);
    //std::cin >> cmd;
    if(cmd == "exit")
      break;
    
    std::vector<std::string> items = parse(cmd);

    for(auto& i : items)
      std::cout << "item: " << i << '\n';
    
    std::vector<std::pair<std::string, bool>> files;
    for(int i=1; i<items.size(); i+=2)
      files.push_back(std::make_pair(items[i], items[i+1] == "1"));
    
    for(auto& f : files)
      std::cout << "file: " << f.first << ' ' << (f.second ? 'V' : 'X') << '\n';
    
    if(items[0] == "relf") {
        gv.reload(files);
    }
    if(items[0] == "rel")
        gv.reload();
    
    else if(items[0] == "vis")
      gv.visibilities(files);
    else if(items[0] == "res")
      gv.resetCamera();
    else if(items[0] == "cent")
      gv.centerCamera();
    else if(items[0] == "close")
      gv.close();
    else if(items[0] == "init")
      gv.init();
    else
      std::cout << "unknown command\n";
  }
  return 0;
}
