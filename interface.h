#pragma once

#include <vector>
#include <string>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#ifndef NOIMPLOT
#include <implot.h>
#endif

struct mainwin_config {
  int width{1024};
  int height{768};
  int posx{31};
  int posy{31};
  std::vector<std::string> recent_files;
};

struct imgui_interface {

  GLFWwindow* window{};
  
  bool init();
  void close();

};
