#pragma once

#include <GL/gl.h>

#include "geom_view.h"

struct fb2way{
  GLuint fb_texture{};
  GLuint fbhl_texture{}; // texture for highlighted objeccts outline render
  GLuint fb_zs_texture{};
  GLuint fb{};
  GLuint fbhl{};
  GLuint fb_shader{};
  GLuint vao{};
  int fbtexloc;
  int fbhltexloc;
  //int fbztexloc;
  int fb_wx_loc;
  int fb_wy_loc;
  int fb_wx{};
  int fb_wy{};
  ~fb2way();
  
  void init(int wx, int wy);
  void resize(int wx, int wy);
  void set_default();
  void render();
  void set_custom();
  void set_custom_hl();
};

struct renderer;
struct object;
struct GLFWwindow;
struct glass_buttons;

void mainloop_pipeline(glass_buttons* btns, fb2way* fb2, renderer* renptr, imgui_interface* iface, object* obj_root, geom_view::UIappearance* = nullptr);
