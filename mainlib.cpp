#include <iostream>
#include <limits>
#include <thread>
#include "draw.h"
#include "interface.h"
#include "object.h"
#include "geom_view.h"
#include "imgui_controls.h"

geom_view::geom_view() {
	iface = new imgui_interface;
}

geom_view::~geom_view() {
	delete iface;
}

void geom_view::thread_func(geom_view* gv) {
  imgui_interface& iface = *(gv->iface);
  iface.init();
  
  gv->obj_root = new object;
  gv->obj_root->name = "root";
  gv->ren_ptr = new renderer;
  renderer& ren = *gv->ren_ptr;
  ren.init(iface.window, gv->obj_root);

  ren.controlPointMoved = gv->controlPointMoved;
  ren.callbackData = gv->callbackData;
  
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

    ren.nocallbacks = ImGui::GetIO().WantCaptureMouse;
    if(ren.nocallbacks)
      glfwSetScrollCallback(iface.window, ImGui_ImplGlfw_ScrollCallback);
    else
      ren.set_callbacks(iface.window);

    static int counter{0};

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ObjectsControl(gv->obj_root, &ren);

    ImGui::Begin("camera");
    ImGui::CameraControl(&ren);

    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Memory: %d Mb", gv->obj_root->memory() /1024/1024);
    ImGui::End();

    ImGui::Render();

    glClearColor(ren.bg_color[0], ren.bg_color[1], ren.bg_color[2], 1.);
    //glClearColor(1., 1., 1., 1.);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //int display_w, display_h;
    //glfwGetFramebufferSize(ren.win, &display_w, &display_h);
    //glViewport(0, 0, display_w, display_h);
    ren.render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
  if(!iface)
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
  controlPointMoved = callback;
  callbackData = data;
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

#ifdef WIN32
HWND geom_view::getNativeWin32Handler() {
	return iface->nativeMSWindowHandler;
}

void geom_view::setParentWin32Handler(HWND _parentMSWindowHandler) {
	iface->parentMSWindowHandler = _parentMSWindowHandler;
}
#endif
