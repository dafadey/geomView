#pragma once

#include <GL/glew.h>
#include <vector>
#include <array>
#include <cfloat>
#include "vectors.h"
struct object;
struct renderer;

struct OGLitem {
  OGLitem();
  virtual void init(renderer* ren);
  virtual void draw(GLfloat* view_matrix, GLfloat* proj_matrix, GLfloat* light_dir) = 0;
  virtual void copyVBOtoDevice() = 0;
  void copyHighlightTexToDevice();
  std::array<vec3f, 2> get_bounds() const;
  virtual ~OGLitem();
  virtual void clear();
  virtual void safeInitHighlight();
  
  GLuint VBO{};
  GLuint vao{};
  GLuint shader{};
  GLuint highlightTex{};
  int highlightTex_loc{-1};
  virtual GLuint VBOstride() const = 0;
  virtual GLuint itemStride() const = 0;

  vec3f geo_min{ FLT_MAX, FLT_MAX, FLT_MAX };
  vec3f geo_max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

  std::vector<GLfloat> VBOdata;
  std::vector<unsigned char> highlighted; // this is a texture used in geomshader
  bool visible = true;
  renderer* ren{};
  GLint verts_location{};
  GLint colors_location{};
  GLint proj_matrix_location{};
  GLint view_matrix_location{};
  GLint aspect_location{};
  GLint mode_loc{};
  GLint texStridePow_loc{};

  bool vboCopied{};
  bool highlightedTextureCopied{};
  int memory() const;
  bool isControlPoints() const;
  void highlight(int i, bool value);
  static int texMaxSizePow;

};

OGLitem* newOGLitem(const std::string& type);

struct OGLtriangles : public OGLitem {
  virtual void init(renderer* ren) override;
  virtual void draw(GLfloat* view_matrix, GLfloat* proj_matrix, GLfloat* light_dir) override;
  virtual void copyVBOtoDevice() override;
  virtual GLuint VBOstride() const override;
  virtual GLuint itemStride() const override;
  void addTriangle(const std::array<vec3f, 3>& coords, const vec3f& color);

  GLint norms_location{};
  GLint light_location{};
  GLint shiny_location{};
  
  GLfloat surface_color[3]{.5f, .5f, .5f};
  GLfloat shiny{.3f};
  
  GLint tris_count{};
};

struct OGLlines : public OGLitem {
  virtual void init(renderer* ren) override;
  virtual void draw(GLfloat* view_matrix, GLfloat* proj_matrix, GLfloat* light_dir) override;
  virtual void copyVBOtoDevice() override;
  virtual GLuint VBOstride() const override;
  virtual GLuint itemStride() const override;
  void addLine(const std::array<vec3f, 2>& coords, const vec3f& color);

  GLint lines_count{};
};

struct OGLvectors : public OGLlines {
  virtual void init(renderer* ren) override;
};

struct OGLpoints : public OGLitem {
  virtual void init(renderer* ren) override;
  virtual void draw(GLfloat* view_matrix, GLfloat* proj_matrix, GLfloat* light_dir) override;
  virtual void copyVBOtoDevice() override;
  virtual GLuint itemStride() const override;
  virtual GLuint VBOstride() const override;

  void addPoint(const vec3f& coords, float radius, const vec3f& color);

  GLint radii_location{};
  GLint points_count{};
};

struct OGLControlPoints : public OGLpoints {
  virtual void init(renderer* ren) override;
};
