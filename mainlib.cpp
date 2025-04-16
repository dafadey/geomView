#include <iostream>
#include <limits>
#include <thread>
#include "draw.h"
#include "interface.h"
#include "object.h"
#include "geom_view.h"
#include "imgui_controls.h"
#include "fb2way.h"
#include "glass_buttons.h"
#include "buttons.png.inl"
#include "tools.h"

geom_view::geom_view() {
	iface = new imgui_interface;
}

geom_view::~geom_view() {
	delete iface;
}

void proc_xyz(int id, glass_button::eaction act, void* dat);

void geom_view::thread_func(geom_view* gv) {
  imgui_interface& iface = *(gv->iface);
  iface.init();
  
  gv->obj_root = new object;
  gv->obj_root->name = "root";
  gv->ren_ptr = new renderer;
  renderer& ren = *gv->ren_ptr;
  ren.init(iface.window, gv->obj_root);

  //ren.controlPointMoved = gv->controlPointMoved;
  //ren.callbackData = gv->callbackData;
  
  gv->reloadLock.lock();
  for(const auto& file : gv->files) {
    if(!file.first.empty())
      load_objects(gv->obj_root, file.first.c_str(), &ren);
  }
  gv->files.clear();
  gv->reloadLock.unlock();

  ren.reset_camera();

  glfwSetWindowUserPointer(iface.window, (void*) &ren);
  
  gv->windowCreationCV.notify_all(); // unlock main thread
  
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
    //glfwPollEvents();
    
    gv->reloadLock.lock();
    if(gv->reloadFlag) {
      std::cout << "files size is " << gv->files.size() << '\n';
      gv->reloadFlag = false;
      reload_files(gv->obj_root, gv->ren_ptr, gv->files);
      gv->files.clear();
    }
    if(gv->changeVisibilityFlag) {
      gv->changeVisibilityFlag = false;
      changeVisibility_for_files(gv->obj_root, gv->ren_ptr, gv->files);
      gv->files.clear();
    }
    if(gv->resetCameraRequest) {
      gv->resetCameraRequest = false;
      gv->ren_ptr->reset_camera();
    }
    gv->reloadLock.unlock();

    mainloop_pipeline(&btns, &fb2, &ren, &iface, gv->obj_root, &gv->appearance);

    glfwSwapBuffers(iface.window);

    glFlush();
  }
  delete gv->obj_root;
  gv->obj_root = nullptr;
  delete gv->ren_ptr;
  gv->ren_ptr = nullptr;
  iface.close();
  gv->windowCreationCV.notify_all(); // unlock main thread, it should be waiting for mainloop to finish by should close flag
  std::cout << "geom view thread finishes\n";
}

void geom_view::init() {
  if(!iface || iface->window)
    return;
  std::unique_lock<std::mutex> ul(windowCreationLock);
  std::thread th(geom_view::thread_func, this);
  th.detach();
  windowCreationCV.wait(ul);
}

void geom_view::close() {
  if(!iface || !iface->window)
    return;
	glfwSetWindowShouldClose(iface->window, 1);
  glfwPostEmptyEvent();
  std::unique_lock<std::mutex> ul(windowCreationLock);
  windowCreationCV.wait(ul);
}

void geom_view::init(const std::vector<std::string>& filenames_) {
  reloadLock.lock();
  files.clear();
  for(const auto& fn : filenames_)
    files.push_back(std::make_pair(fn, true));
  reloadLock.unlock();
  init();
}

void geom_view::init(const std::string& filename) {
  std::vector<std::string> filenames;
  filenames.push_back(filename);
  init(filenames);
}

void geom_view::reload(bool resetCam) {
  if(!ren_ptr)
    init();
  reloadLock.lock();
  reloadFlag = true;
  files.clear();
  if(obj_root) {
    for(auto& item : obj_root->children())
      files.push_back(std::make_pair(item->name, true));
  }
  resetCameraRequest = resetCam;
  reloadLock.unlock();
  glfwPostEmptyEvent();  

  //no need for that stuff with context ownership management since it is more stable just to send request to mainthread. EmptyEvent allows to process request -- that is it!
  //glfwMakeContextCurrent(ren_ptr->win); // typically calling thread differs from one that creates geomView main window so to make GL functions work properly we have to set current GL context to one created during main window creation.
  //::reload(obj_root, ren_ptr);
}

void geom_view::reload(const std::vector<std::pair<std::string, bool>>& files_, bool resetCam) {
  if(!ren_ptr)
    init();
  while(true) {
    std::unique_lock<std::mutex> ul(reloadLock);
    if(files.size()==0)
      break;
  }
  reloadLock.lock();
  reloadFlag = true;
  files = files_;
  resetCameraRequest = resetCam;
  reloadLock.unlock();
  glfwPostEmptyEvent();
}

void geom_view::visibilities(const std::vector<std::pair<std::string, bool>>& files_) {
  if(!ren_ptr)
    init();
  
  while(true) {
    std::unique_lock<std::mutex> ul(reloadLock);
    if(files.size()==0)
      break;
  }
  
  reloadLock.lock();
  changeVisibilityFlag = true;
  files = files_;
  reloadLock.unlock();
  glfwPostEmptyEvent();
}

void geom_view::setCallBack(void* data, void (*callback)(void*, std::vector<std::string>& sId, double x, double y, double z)) {
  if(ren_ptr) {
    ren_ptr->controlPointMoved = callback;
    ren_ptr->callbackData = data;
  }
  //controlPointMoved = callback;
  //callbackData = data;
}

void geom_view::setSelectCallBack(void* data, void (*callback)(void*, const std::vector<std::tuple<std::vector<std::string>, size_t, float>>&)) {
  if(ren_ptr) {
    ren_ptr->selected = callback;
    ren_ptr->selectedData = data;
  }
  //controlPointMoved = callback;
  //selectedData = data;
  
}


void geom_view::centerCamera() {
  if(ren_ptr)
    ren_ptr->center_camera();
  glfwPostEmptyEvent();
}

void geom_view::resetCamera() {
  if(ren_ptr)
    ren_ptr->reset_camera();
  glfwPostEmptyEvent();
}  

#ifdef _WIN32
HWND geom_view::getNativeWin32Handler() {
	return iface->nativeMSWindowHandler;
}

void geom_view::setParentWin32Handler(HWND _parentMSWindowHandler) {
	iface->parentMSWindowHandler = _parentMSWindowHandler;
}
#endif

void geom_view::addCustomControl(const std::shared_ptr<geom_view_control>& cc) {
  iface->custom_controls.push_back(cc);
}

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

std::vector<std::string> geom_view::tokenize(const std::string& in, const std::string& delim) {
  return split(in, delim);
}
