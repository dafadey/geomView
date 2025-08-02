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
#include "geom_view.h"

#ifdef GLEWINIT_INTERNAL
  extern GLenum __stdcall glewInit(void);
#endif

struct GLFWwindow;

struct object;

struct renderer {
  enum e_mouse_state{HOVER, ROTATE, PAN, ZOOM, SELECT_DRAG};
  enum e_draw_mode{NORMAL=0, HIGHLIGHT=1};
  
  renderer(geom_view::ViewControls&);

  struct origin {
    bool show{};
    void clear();
    void init(renderer* ren);
    OGLvectors axes;
    float size{.3f};
  };
  
  origin o;

  geom_view::ViewControls* viewControls_ptr{};
  
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

  e_draw_mode draw_mode{e_draw_mode::NORMAL};
  
  //GLFWwindow* win;
  std::array<int,2>* outputGeo_ptr{};
  
  e_mouse_state mouse_state{HOVER};
  double mouse_pos[2];

  float scale;

  bool nocallbacks{};

  bool init(object*);
  
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
  
  //callback and it's data on control point move
  void (*controlPointMoved)(void*, std::vector<std::string>& sId, double x, double y, double z) {nullptr};
  void* callbackData {nullptr};
  //callback and it's data for selection (RMB single click)
  void (*selected)(void*, const std::vector<std::tuple<std::vector<std::string>, size_t, float>>&) {nullptr};
  void* selectedData {nullptr};

  bool need_bg_color_picker{};

  mutable std::vector<std::tuple<const object*, GLfloat, size_t>> selectionResults;
  mutable float selectedPointRadius{.0f};
  mutable vec3f selectedPointPos0;
  mutable double xPressPos, yPressPos;

};

