#include <iostream>
#include <limits>
#include <thread>
#include "draw.h"
#include "interface.h"
#include "object.h"
#include "geom_view.h"
#include "imgui_controls.h"

static void thread_func(geom_view* gv, std::string filename) {
  imgui_interface iface;
  iface.init();
    
  gv->obj_root = new object;
  gv->obj_root->name = "root";
  gv->ren_ptr = new renderer;
  renderer& ren = *gv->ren_ptr;
  ren.init(iface.window, gv->obj_root);

  ren.controlPointMoved = gv->controlPointMoved;
  ren.callbackData = gv->callbackData;
  
  if(!filename.empty())
    load_objects(gv->obj_root, filename.c_str(), &ren);
 
  ren.reset_camera();

  glfwSetWindowUserPointer(iface.window, (void*) &ren);
    
  while (!glfwWindowShouldClose(iface.window))  {
    glfwWaitEvents();
    //glfwPollEvents();
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
  iface.close();
}

void geom_view::init(const std::string& filename) {
  std::thread th(thread_func, this, filename);
  th.detach();
}

void geom_view::reload() {
  ::reload(obj_root, ren_ptr);
}

void geom_view::setCallBack(void* data, void (*callback)(void*, std::vector<std::string>& sId, double x, double y, double z)) {
  if(ren_ptr) {
    ren_ptr->controlPointMoved = callback;
    ren_ptr->callbackData = data;
  }
  controlPointMoved = callback;
  callbackData = data;
}
