#pragma once

#include <GL/glew.h>
#include <vector>
#include <array>
#include <cfloat>
#include "vectors.h"

struct renderer;

struct OGLitem {
  OGLitem();
  virtual void init(renderer* ren) = 0;
  virtual void draw(GLfloat* view_matrix, GLfloat* proj_matrix, GLfloat* light_dir) = 0;
  virtual void copyVBOtoDevice() = 0;
  std::array<vec3f, 2> get_bounds() const;
  virtual ~OGLitem();

  GLuint VBO{};
  GLuint vao{};
  GLuint shader{};
  virtual GLuint VBOstride() const = 0;

  vec3f geo_min{ FLT_MAX, FLT_MAX, FLT_MAX };
  vec3f geo_max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

  std::vector<GLfloat> VBOdata;
  bool visible = true;
  renderer* ren{};

  bool vboCopied{};
  int memory();
};

OGLitem* newOGLitem(const std::string& type);

struct OGLtriangles : public OGLitem {
  virtual void init(renderer* ren) override;
  virtual void draw(GLfloat* view_matrix, GLfloat* proj_matrix, GLfloat* light_dir) override;
  virtual void copyVBOtoDevice() override;
  virtual GLuint VBOstride() const override;
  void addTriangle(const std::array<vec3f, 3>& coords, const vec3f& color);

  GLint verts_location{};
  GLint norms_location{};
  GLint colors_location{};
  GLint light_location{};
  GLint shiny_location{};
  GLint proj_matrix_location{};
  GLint view_matrix_location{};
  
  GLfloat surface_color[3]{.5f, .5f, .5f};
  GLfloat shiny{0.3};
  
  GLint tris_count{};
};

struct OGLlines : public OGLitem {
  virtual void init(renderer* ren) override;
  virtual void draw(GLfloat* view_matrix, GLfloat* proj_matrix, GLfloat* light_dir) override;
  virtual void copyVBOtoDevice() override;
  virtual GLuint VBOstride() const override;
  void addLine(const std::array<vec3f, 2>& coords, const vec3f& color);

  GLint verts_location{};
  GLint colors_location{};
  GLint proj_matrix_location{};
  GLint view_matrix_location{};

  GLint lines_count{};
};

struct OGLpoints : public OGLitem {
  virtual void init(renderer* ren) override;
  virtual void draw(GLfloat* view_matrix, GLfloat* proj_matrix, GLfloat* light_dir) override;
  virtual void copyVBOtoDevice() override;
  virtual GLuint VBOstride() const override;

  void addPoint(const vec3f& coords, float radius, const vec3f& color);

  GLint verts_location{};
  GLint radii_location{};
  GLint colors_location{};
  GLint proj_matrix_location{};
  GLint view_matrix_location{};
  GLint aspect_location{};

  GLint points_count{};
};
