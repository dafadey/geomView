#pragma once

#include <vector>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#ifndef NOIMPLOT
#include <implot.h>
#endif

#include "custom_controls.h"

#define MAINWIN_WIDTH_DEFAULT 1024
#define MAINWIN_HEIGHT_DEFAULT 768
#define MAINWIN_POSX_DEFAULT 31
#define MAINWIN_POSY_DEFAULT 31

struct mainwin_config {
  int width{MAINWIN_WIDTH_DEFAULT};
  int height{MAINWIN_HEIGHT_DEFAULT};
  int posx{MAINWIN_POSX_DEFAULT};
  int posy{MAINWIN_POSY_DEFAULT};
  std::vector<std::string> recent_files;
  bool valid() const;
};

struct imgui_interface {
  GLFWwindow* window{};
#ifdef _WIN32
  HWND nativeMSWindowHandler = 0;
  HWND parentMSWindowHandler = 0;
#endif
  
  std::vector<std::shared_ptr<geom_view_control>> custom_controls;
  
  bool inited{};

  void CustomControls();
  bool init(bool hideWindow = false);
  void close();
};
