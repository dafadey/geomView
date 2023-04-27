#include <iostream>
#include <limits>
#include "draw.h"
#include "interface.h"
#include "object.h"
#include "imgui_controls.h"

int main(int argc, char* argv[]) {
  std::cout << "Hallo!\n";

  if(argc == 1) {
    std::cout << "set input files" << std::endl;
    return 0;
  }
    
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

    ImGui::ObjectsControl(obj_root, &ren);

    ImGui::Begin("camera");
    ImGui::CameraControl(&ren);

    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Memory: %d Mb", obj_root->memory() /1024/1024);
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
  delete obj_root;
  iface.close();
}
