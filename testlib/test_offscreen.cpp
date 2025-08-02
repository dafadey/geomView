#include <iostream>
#include <fstream>
#include <vector>

#include "geom_view.h"

int main(int argc, char** argv) {
  std::string file = "../sample.txt";
  if(argc > 1)
    file = argv[1];
  geom_view gv;
  gv.setOffsсreen(640, 480);
  gv.init(file.c_str());
  gv.resetCamera();
  gv.makeShot();
  int nx,ny;
  std::vector<unsigned char> buff;
  gv.getBuffer(nx, ny, buff);
  std::ofstream of("buff.dat");
  of << nx << '\n' << ny << '\n';
  for(int i=0;i<nx*ny;i++)
    of << (int) buff[i*3] << ' ' << (int) buff[i*3+1] << ' ' << (int) buff[i*3+2] << '\n';
  of.close();
  return 0;
}
