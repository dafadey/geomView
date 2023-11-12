#include <iostream>
#include <fstream>
#include "../geom_view.h"
#include "../vectors.cpp" //yep lets include whole source with header inside

struct geo {
  std::array<vec3f, 3> controlPts;
  bool origin = false;
};

void dumpPoint(std::ostream& o, const vec3f& v) {
  o << "(" << v[0] << ", " << v[1] << ", " << v[2] << ") 13 (1, 0, 0)\n";
}

void dumpLine(std::ostream& o, const vec3f& v0, const vec3f& v1) {
  o << "(" << v0[0] << ", " << v0[1] << ", " << v0[2] << ") (" << v1[0] << ", " << v1[1] << ", " << v1[2] << ") (1, 1, 0)\n";
}

void dump_geo(geo g) {
  std::ofstream of("triangle.txt");
  of << "control_points: c\n";
  for(int i=0;i<3;i++)
    dumpPoint(of, g.controlPts[i]);
  of << "lines: a\n";
  for(int i=0;i<3;i++)
    dumpLine(of, g.controlPts[i], g.controlPts[(i+1)%3]);
  of << "points: medial_point\n";
  dumpPoint(of, (1.f/3.f)*(g.controlPts[0]+g.controlPts[1]+g.controlPts[2]));
  if(g.origin) {
    of << "vectors: O\n";
    of << "(0,0,0) (1,0,0) (0.93, 0.13, 0.11)\n";
    of << "(0,0,0) (0,1,0) (0.13, 0.83, 0.17)\n";
    of << "(0,0,0) (0,0,1) (0.11, 0.13, 0.87)\n";
  }
  of.close();
}

void moveControl(void* callback_data, std::vector<std::string>& sId, double x, double y, double z) {
  auto data = static_cast<std::array<void*,2>*>(callback_data);
  geo* g = static_cast<geo*>((*data)[1]);
  geom_view* gv = static_cast<geom_view*>((*data)[0]);
  
  int id = atoi(sId.back().c_str());
  g->controlPts[id] = vec3f{x,y,z};
  dump_geo(*g);
  gv->reload();
}

int main() {
  geo g;

  g.controlPts[0] = vec3f{0,0,0.1};
  g.controlPts[1] = vec3f{3,0,0.2};
  g.controlPts[2] = vec3f{0,1.7,0.3};
  dump_geo(g);

  geom_view gv;
  gv.init("triangle.txt");

  std::array<void*,2> callback_data {(void*) &gv, (void*) &g};

  gv.setCallBack((void*) &callback_data, &moveControl);

  std::string cmd;
  while(cmd != "exit") {
    std::cin >> cmd;
    if(cmd == "exit")
      break;
    else if(cmd == "originON")
      g.origin = true;
    else if(cmd == "originOFF")
      g.origin = false;
    else if(cmd == "reload")
      gv.reload();
    else {
      std::cout << "unknown command\n";
      continue;
    }
    dump_geo(g);
    gv.reload();
  }
  return 0;
}
