#include <iostream>

#include "OGLitem.h"
#include "draw.h"
#include "timer.h"

#include "GLFW/glfw3.h"

static std::array<float, 2> getAspect(renderer* ren) {
  std::array<float, 2> aspect{1.f, 1.f};
  if(ren) {
    int win_geo[2];
    glfwGetWindowSize(ren->win, win_geo, &win_geo[1]);
    for(int i=0;i<2;i++)
      aspect[i] = 2.f/static_cast<float>(win_geo[i]);
  }
  return aspect;
}

bool OGLitem::isControlPoints() const {
  return dynamic_cast<const OGLControlPoints*>(this);
}

OGLitem::OGLitem() : vao(0), VBO(0), shader(0) {}

OGLitem::~OGLitem() {
  if(VBO)
    glDeleteBuffers(1, &VBO);
  if(vao)
    glDeleteVertexArrays(1, &vao);
}

int OGLitem::memory() const {
  return VBOdata.size() * sizeof(GLfloat);
}

std::array<vec3f, 2> OGLitem::get_bounds() const {
  return std::array<vec3f, 2>{geo_min, geo_max};
}

OGLitem* newOGLitem(const std::string& type) {
  if(type == "triangles") {
    return new OGLtriangles();
  } else if (type == "lines") {
    return new OGLlines();
  } else if (type == "vectors") {
    return new OGLvectors();
  } else if (type == "points") {
    return new OGLpoints();
  } else if (type == "control_points") {
    return new OGLControlPoints();
  } else {
    std::cout << "newOGLitem: unknown type " << type << '\n';
    return nullptr;
  }
}

void OGLitem::clear() {
  geo_min = vec3f{ FLT_MAX, FLT_MAX, FLT_MAX };
  geo_max = vec3f{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
  VBOdata.clear();
  vboCopied = false;
}

void OGLtriangles::init(renderer* ren_) {
  ren = ren_;

  if(!vao) {
    glGenVertexArrays(1, &vao);
    if (vao)
      glBindVertexArray(vao);
    else
      std::cout << "failed to get vao\n";
  } else
    std::cout << "WARNING: VAO is already initialized to " << vao << '\n';
  
  if(!VBO)
    glGenBuffers(1, &VBO);
  else
    std::cout << "WARNING: VBO is already initialized to " << VBO << '\n';

  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  shader = ren->getShader("sha.vs", "sha.fs");
  
  verts_location = glGetAttribLocation(shader, "vertex_pos");
  norms_location = glGetAttribLocation(shader, "normal_pos");
  colors_location = glGetAttribLocation(shader, "triangle_color");

  light_location = glGetUniformLocation(shader, "light_dir");
  proj_matrix_location = glGetUniformLocation(shader, "proj_matrix");
  view_matrix_location = glGetUniformLocation(shader, "view_matrix");
  shiny_location = glGetUniformLocation(shader, "shiny");


  std::cout << "\tattr: verts_location=" << verts_location << '\n';
  std::cout << "\tattr: norms_location=" << norms_location << '\n';
  std::cout << "\tattr: color_location=" << colors_location << '\n';

  std::cout << "\tuniform: light_location=" << light_location << '\n';
  std::cout << "\tuniform: shiny_location=" << shiny_location << '\n';
  std::cout << "\tuniform: proj_matrix_location=" << proj_matrix_location << '\n';
  std::cout << "\tuniform: view_matrix_location=" << view_matrix_location << '\n';
  glBindVertexArray(0);
}

GLuint OGLtriangles::VBOstride() const {
  return 9;
}

void OGLtriangles::copyVBOtoDevice() {
  if (vboCopied)
    return;

  timer tim("OGLsurface::update_model time is");
  if (VBO == 0) {
    std::cerr << "OGLtriangles::cannot update model - VBO is not generated, call init() for renderer\n";
    return;
  }

  if(!VBOdata.size()) {
    vboCopied = true;
    std::cerr << "tri: no data to copy to device VBO, VBOdata is empty\n";
    return;
  }

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, VBOdata.size() * sizeof(GLfloat), VBOdata.data(), GL_STATIC_DRAW);

  int size_of_OGL_vertex = 3 * sizeof(float) + 3 * sizeof(float) + 3 * sizeof(float); // 3 coords, 3 normals, 3 colors

  tris_count = VBOdata.size() * sizeof(float) / size_of_OGL_vertex / 3;

//memory: 3 coords, 3 normals, 3 colors
  glVertexAttribPointer(verts_location,3,GL_FLOAT,GL_FALSE, size_of_OGL_vertex, (void*)0);
  glVertexAttribPointer(norms_location, 3, GL_FLOAT, GL_FALSE, size_of_OGL_vertex, (void*)(3*sizeof(float)));
  glVertexAttribPointer(colors_location, 3, GL_FLOAT, GL_FALSE, size_of_OGL_vertex, (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(verts_location);
  glEnableVertexAttribArray(norms_location);
  glEnableVertexAttribArray(colors_location);

  glBindVertexArray(0);

  vboCopied = true;
}

void OGLtriangles::addTriangle(const std::array<vec3f,3>& coords, const vec3f& color)
{
  vec3f a = coords[1] - coords[0];
  vec3f b = coords[2] - coords[0];
  vec3f n = cross_prod(a, b);
  normalize(n);

  for(auto& pt : coords) {
    for(int c=0; c<3; c++) {
      VBOdata.push_back(pt[c]);
      geo_min[c] = geo_min[c] < pt[c] ? geo_min[c] : pt[c];
      geo_max[c] = geo_max[c] > pt[c] ? geo_max[c] : pt[c];
    }
    for(int c=0; c < 3; c++)
      VBOdata.push_back(n[c]);
    for (int c = 0; c < 3; c++)
      VBOdata.push_back(color[c]);
  }
  vboCopied = false;
}

void OGLtriangles::draw(GLfloat* view_matrix, GLfloat* proj_matrix, GLfloat* light_dir) {
  copyVBOtoDevice();

  glUseProgram(shader);

  glEnable(GL_DEPTH_TEST);
  
  glUniformMatrix4fv(view_matrix_location, 1, false, view_matrix);
  glUniformMatrix4fv(proj_matrix_location, 1, false, proj_matrix);

  glUniform3fv(light_location, 1, light_dir);
  glUniform1f(shiny_location, shiny);

  glBindVertexArray(vao);

  glDrawArrays(GL_TRIANGLES, 0, tris_count * 3);
  glBindVertexArray(0);
  glUseProgram(0);
}






void OGLlines::init(renderer* ren_) {
  ren = ren_;

  if (!vao) {
    glGenVertexArrays(1, &vao);
    if (vao)
      glBindVertexArray(vao);
    else
      std::cout << "failed to get vao err#"<< glGetError() << "\n";
  }
  else
    std::cout << "WARNING: VAO is already initialized to " << vao << '\n';

  if (!VBO)
    glGenBuffers(1, &VBO);
  else
    std::cout << "WARNING: VBO is already initialized to " << VBO << '\n';

  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  shader = ren->getShader("sha_line.vs", "sha.fs");

  verts_location = glGetAttribLocation(shader, "vertex_pos");
  colors_location = glGetAttribLocation(shader, "line_color");

  proj_matrix_location = glGetUniformLocation(shader, "proj_matrix");
  view_matrix_location = glGetUniformLocation(shader, "view_matrix");
  aspect_location = glGetUniformLocation(shader, "aspect");

  std::cout << "\tattr: verts_location=" << verts_location << '\n';
  std::cout << "\tattr: colors_location=" << colors_location << '\n';

  std::cout << "\tuniform: proj_matrix_location=" << proj_matrix_location << '\n';
  std::cout << "\tuniform: view_matrix_location=" << view_matrix_location << '\n';
  glBindVertexArray(0);
}

void OGLvectors::init(renderer* ren_) {
  OGLlines::init(ren_);
  if (vao) {
    glBindVertexArray(vao);
    shader = ren->getShader("sha_vector.vs", "sha.fs", "sha_vector.gs");
    verts_location = glGetAttribLocation(shader, "vertex_pos");
    colors_location = glGetAttribLocation(shader, "line_color");

    proj_matrix_location = glGetUniformLocation(shader, "proj_matrix");
    view_matrix_location = glGetUniformLocation(shader, "view_matrix");
    aspect_location = glGetUniformLocation(shader, "aspect");
    glBindVertexArray(0);
  } else
    std::cout << "failed to get vao\n";
}

GLuint OGLlines::VBOstride() const {
  return 6;
}

void OGLlines::copyVBOtoDevice() {
  if (vboCopied)
    return;

  timer tim("OGLsurface::update_model time is");
  if (VBO == 0) {
    std::cerr << "OGLlines::cannot update model - VBO is not generated, call init() for renderer\n";
    return;
  }

  if (!VBOdata.size()) {
    vboCopied = true;
    std::cerr << "lines: no data to copy to device VBO, VBOdata is empty\n";
    return;
  }

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, VBOdata.size() * sizeof(GLfloat), VBOdata.data(), GL_STATIC_DRAW);

  int size_of_OGL_vertex = 3 * sizeof(float) + 3 * sizeof(float); // 3 coords, 3 colors

  lines_count = VBOdata.size() * sizeof(float) / size_of_OGL_vertex / 2;

  //memory: 3 coords, 3 colors
  glVertexAttribPointer(verts_location, 3, GL_FLOAT, GL_FALSE, size_of_OGL_vertex, (void*)0);
  glVertexAttribPointer(colors_location, 3, GL_FLOAT, GL_FALSE, size_of_OGL_vertex, (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(verts_location);
  glEnableVertexAttribArray(colors_location);

  glBindVertexArray(0);

  vboCopied = true;
}

void OGLlines::addLine(const std::array<vec3f, 2>& coords, const vec3f& color)
{
  for (auto& pt : coords) {
    for (int c = 0; c < 3; c++) {
      VBOdata.push_back(pt[c]);
      geo_min[c] = geo_min[c] < pt[c] ? geo_min[c] : pt[c];
      geo_max[c] = geo_max[c] > pt[c] ? geo_max[c] : pt[c];
    }
    for (int c = 0; c < 3; c++)
      VBOdata.push_back(color[c]);
  }
  vboCopied = false;
}

void OGLlines::draw(GLfloat* view_matrix, GLfloat* proj_matrix, GLfloat* light_dir) {
  copyVBOtoDevice();

  glUseProgram(shader);

  glEnable(GL_DEPTH_TEST);

  glUniformMatrix4fv(view_matrix_location, 1, false, view_matrix);
  glUniformMatrix4fv(proj_matrix_location, 1, false, proj_matrix);
  glUniform2fv(aspect_location, 1, getAspect(ren).data());

  glBindVertexArray(vao);

  glDrawArrays(GL_LINES, 0, lines_count * 2);
  glBindVertexArray(0);
  glUseProgram(0);
}











void OGLpoints::init(renderer* ren_) {
  ren = ren_;

  if (!vao) {
    glGenVertexArrays(1, &vao);
    if(vao)
      glBindVertexArray(vao);
    else
      std::cout << "failed to get vao\n";
  }
  else
    std::cout << "WARNING: VAO is already initialized to " << vao << '\n';

  if (!VBO)
    glGenBuffers(1, &VBO);
  else
    std::cout << "WARNING: VBO is already initialized to " << VBO << '\n';

  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  shader = ren->getShader("sha_circle.vs", "sha.fs", "sha_circle.gs");

  verts_location = glGetAttribLocation(shader, "point_pos");
  radii_location = glGetAttribLocation(shader, "radius");
  colors_location = glGetAttribLocation(shader, "point_color");

  proj_matrix_location = glGetUniformLocation(shader, "proj_matrix");
  view_matrix_location = glGetUniformLocation(shader, "view_matrix");
  aspect_location = glGetUniformLocation(shader, "aspect");

  std::cout << "\tattr: verts_location=" << verts_location << '\n';
  std::cout << "\tattr: radii_location=" << radii_location << '\n';
  std::cout << "\tattr: colors_location=" << colors_location << '\n';

  std::cout << "\tuniform: proj_matrix_location=" << proj_matrix_location << '\n';
  std::cout << "\tuniform: view_matrix_location=" << view_matrix_location << '\n';
  std::cout << "\tuniform: aspect_location=" << aspect_location << '\n';
  glBindVertexArray(0);
}


void OGLControlPoints::init(renderer* ren_) {
  OGLpoints::init(ren_);
  if (vao) {
    glBindVertexArray(vao);
    shader = ren->getShader("sha_circle.vs", "sha.fs", "sha_cp.gs");
    verts_location = glGetAttribLocation(shader, "point_pos");
    radii_location = glGetAttribLocation(shader, "radius");
    colors_location = glGetAttribLocation(shader, "point_color");

    proj_matrix_location = glGetUniformLocation(shader, "proj_matrix");
    view_matrix_location = glGetUniformLocation(shader, "view_matrix");
    aspect_location = glGetUniformLocation(shader, "aspect");
    glBindVertexArray(0);
  } else
    std::cout << "failed to get vao\n";
}


GLuint OGLpoints::VBOstride() const {
  return 7;
}

void OGLpoints::copyVBOtoDevice() {
  if (vboCopied)
    return;

  timer tim("OGLsurface::update_model time is");
  if (VBO == 0) {
    std::cerr << "OGLpoints::cannot update model - VBO is not generated, call init() for renderer\n";
    return;
  }

  if (!VBOdata.size()) {
    vboCopied = true;
    std::cerr << "points: no data to copy to device VBO, VBOdata is empty\n";
    return;
  }

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, VBOdata.size() * sizeof(GLfloat), VBOdata.data(), GL_STATIC_DRAW);

  int size_of_OGL_vertex = 3 * sizeof(float) + sizeof(float) + 3 * sizeof(float); // 3 coords, 1 raduis, 3 colors

  points_count = VBOdata.size() * sizeof(float) / size_of_OGL_vertex;

  //memory: 3 coords, 1 radius, 3 colors
  glVertexAttribPointer(verts_location, 3, GL_FLOAT, GL_FALSE, size_of_OGL_vertex, (void*)0);
  glVertexAttribPointer(radii_location, 1, GL_FLOAT, GL_FALSE, size_of_OGL_vertex, (void*)(3 * sizeof(float)));
  glVertexAttribPointer(colors_location, 3, GL_FLOAT, GL_FALSE, size_of_OGL_vertex, (void*)(4 * sizeof(float)));
  glEnableVertexAttribArray(verts_location);
  glEnableVertexAttribArray(radii_location);
  glEnableVertexAttribArray(colors_location);

  glBindVertexArray(0);

  vboCopied = true;
}

void OGLpoints::addPoint(const vec3f& pt, float radius, const vec3f& color)
{
  for (int c = 0; c < 3; c++) {
    VBOdata.push_back(pt[c]);
    geo_min[c] = geo_min[c] < pt[c] ? geo_min[c] : pt[c];
    geo_max[c] = geo_max[c] > pt[c] ? geo_max[c] : pt[c];
  }
  VBOdata.push_back(radius);
  for (int c = 0; c < 3; c++)
    VBOdata.push_back(color[c]);
  vboCopied = false;
}

void OGLpoints::draw(GLfloat* view_matrix, GLfloat* proj_matrix, GLfloat* light_dir) {
  copyVBOtoDevice();

  glUseProgram(shader);

  glEnable(GL_DEPTH_TEST);

  glUniformMatrix4fv(view_matrix_location, 1, false, view_matrix);
  glUniformMatrix4fv(proj_matrix_location, 1, false, proj_matrix);
  glUniform2fv(aspect_location, 1, getAspect(ren).data());

  glBindVertexArray(vao);

  glDrawArrays(GL_POINTS, 0, points_count);
  glBindVertexArray(0);
  glUseProgram(0);
}

