#include <iostream>
#include <limits>
#include "draw.h"
#include "interface.h"
#include "object.h"
#include "imgui_controls.h"
#include "fb2way.h"
#include "glass_buttons.h"
#include "buttons.png.inl"
#include "geom_view.h"

void proc_xyz(int id, glass_button::eaction act, void* dat) {
  renderer* ren = (renderer*) dat;
  //std::cout << "xyz button " << id << " experienced action " << act << " rnd=" << rand() << '\n';
  vec3f dir = ren->fp_pos - ren->cam_pos;
  vec3f up = ren->cam_up;
  normalize(dir);
  normalize(up);
  if(id==0) {
    dir = vec3f{1,0,0};
    up = vec3f{0,1,0};
  }
  if(id==1) {
    dir = vec3f{-1,0,0};
    up = vec3f{0,1,0};
  }
  if(id==2) {
    dir = vec3f{0,1,0};
    up = vec3f{1,0,0};
  }
  if(id==3) {
    dir = vec3f{0,-1,0};
    up = vec3f{1,0,0};
  }
  if(id==4) {
    dir = vec3f{0,0,1};
    up = vec3f{1,0,0};
  }
  if(id==5) {
    dir = vec3f{0,0,-1};
    up = vec3f{1,0,0};
  }
  //rotate 90
  if(id==6)
    up = cross_prod(up, dir);
  if(id==7)
    up = cross_prod(dir, up);
  //center and reset
  if(id==8)
    ren->center_camera();
  if(id==9)
    ren->reset_camera();
  //background color
  if(id==10)
    ren->need_bg_color_picker = !ren->need_bg_color_picker;

  if(id < 8 && dir*dir) {
    ren->cam_pos = ren->fp_pos - std::sqrt((ren->cam_pos - ren->fp_pos) * (ren->cam_pos - ren->fp_pos)) * dir;
    ren->cam_up = up;
  }
  glfwPostEmptyEvent();
}

int main(int argc, char* argv[]) {
  std::cout << "Hallo!\n";

  imgui_interface iface;
  iface.init();
    
  object* obj_root = new object;
  obj_root->name = "root";
  renderer ren;
  ren.init(iface.window, obj_root);

  for(int i=1; i<argc; i++)
    load_objects(obj_root, argv[i], &ren);
 
  ren.reset_camera();

  glfwSetWindowUserPointer(iface.window, (void*) &ren);
  
  fb2way fb2;
  
  int wx, wy;
  glfwGetWindowSize(iface.window, &wx, &wy);
  fb2.init(wx, wy);
  
  glass_buttons btns(iface.window);
  btns.init(buttons_png, buttons_png_len);
  //btns.init(nullptr, 0);
  btns.bgtex = fb2.fb_texture;
  
  for(int i=0;i<11;i++)
    btns.addButton(glass_button(i, 10+(10+32)*i, 10, 32, 32, 32*i, 0, 32*(i+1), 32, proc_xyz, &ren));
  
  while (!glfwWindowShouldClose(iface.window))  {
    glfwWaitEvents();
    mainloop_pipeline(&btns, &fb2, &ren, iface.window, obj_root);
    glfwSwapBuffers(iface.window);
    //glFlush();
  }
  delete obj_root;
  iface.close();
}
