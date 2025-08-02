#include <iostream>
#include "geom_view.h"

int main(int argc, char* argv[]) {
  std::cout << "Hallo this is geomView!\n";

  geom_view gv;
  //gv.viewControls.rotateButton = geom_view::LEFT_BUTTON;
  //gv.viewControls.panButton = geom_view::RIGHT_BUTTON;
  //gv.viewControls.selectButton = geom_view::MIDDLE_BUTTON;

  gv.runInThread = false;
  
  std::vector<std::string> files;
  for(int i=1; i<argc; i++)
    files.push_back(argv[i]);

  gv.init(files);
  
  return 0;
}
