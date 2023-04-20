#pragma once

#include "OGLitem.h"
#include "vectors.h"
#include <list>

struct OGLitem;

struct object {
  std::string name;
  
  ~object();

  void setItemsVisible(bool setting);
  bool visible{true};
  OGLitem* item{nullptr};
  std::list<object*> children;
  int memory();
};

void load_objects(object* obj_root, const std::string& file, renderer* ren);

