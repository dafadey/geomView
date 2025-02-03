#pragma once

#include <GL/gl.h>

#include "geom_view.h"

struct fb2way{
  GLuint fb_texture{};
  GLuint fb_zs_texture{};
  GLuint fb{};
  GLuint fb_shader{};
  GLuint vao{};
  int fbtexloc;
  //int fbztexloc;
  int fb_wx{};
  int fb_wy{};
  ~fb2way();
  
  void init(int wx, int wy);
  void resize(int wx, int wy);
  void set_default();
  void render();
  void set_custom();
};

struct renderer;
struct object;
struct GLFWwindow;
struct glass_buttons;

void mainloop_pipeline(glass_buttons* btns, fb2way* fb2, renderer* renptr, GLFWwindow* window, object* obj_root, geom_view::UIappearance* = nullptr);
