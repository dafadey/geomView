#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#ifndef GLEWINIT_INTERNAL
  #include <GL/glew.h>
#endif

#include <array>
#include <vector>
#include <cfloat>
#include <string>
#include <map>
#include "OGLitem.h"
#include "vectors.h"

#ifdef GLEWINIT_INTERNAL
  extern GLenum __stdcall glewInit(void);
#endif

struct GLFWwindow;

struct object;

struct renderer {
  enum e_mouse_state{HOVER, ROTATE, PAN, ZOOM, SELECT_DRAG};
    
  object* obj{};

  std::map<std::array<std::string,3>, GLuint> shaders;
  
  vec3f cam_pos{};
  vec3f fp_pos{};
  vec3f cam_up{};
  vec3f light_dir{};

  std::array<GLfloat,16> view_matrix;
  std::array<GLfloat,16> proj_matrix;
 
  float mask_r_x{.1f};
  float mask_r_y{.1f};
  bool mask_erases{true};

  GLFWwindow* win;
  
  e_mouse_state mouse_state{HOVER};
  double mouse_pos[2];

  float scale;

  bool nocallbacks{};

  bool init(GLFWwindow*, object*);
  
  void render();
  
  vec2f project(const vec3f&) const;
  
  void select(bool onlyControlPoints = false) const;

  void set_callbacks(GLFWwindow* window);
  
  GLuint getShader(const std::string& vs_name, const std::string& fs_name, const std::string& gs_name = "");

  vec3f center_camera();

  void reset_camera();

  std::vector<OGLitem*> get_items();
  
  bool parallel=true;
  
  std::array<GLfloat, 3> bg_color {.3f, .3f, .3f};
  
  void (*controlPointMoved)(void*, std::vector<std::string>& sId, double x, double y, double z) {nullptr};
  void* callbackData {nullptr};

  mutable std::vector<std::tuple<const object*, GLfloat, size_t>> selectionResults;
  mutable float selectedPointRadius{.0f};
  mutable vec3f selectedPointPos0;
  mutable double xPressPos, yPressPos;

};

