#pragma once

#include "OGLitem.h"
#include "vectors.h"
#include <list>

struct OGLitem;

struct object {
  std::string name;
 
  ~object();

  std::vector<std::string> fullName() const;
  void setItemsVisible(bool setting);
  bool visible{true};
  object* parent{nullptr};
  int memory() const;
  void addChild(object* c);
  std::list<object*>::const_iterator addChild(std::list<object*>::const_iterator it, object* c);
  void removeChild(object* c);
  void removeChild(std::list<object*>::const_iterator it);
  const std::list<object*>& children() const;
  OGLitem* item{nullptr};

private:
  std::list<object*> m_children;
};

bool load_objects(object* obj_root, const std::string& file, renderer* ren);

void reload(object* obj_root, renderer* ren);
