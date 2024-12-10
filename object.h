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
  void removeAllChildren();
  const std::list<object*>& children() const;
  std::list<object*>& children();
  OGLitem* item{nullptr};

private:
  std::list<object*> m_children;
};

//always loads, always adds new object to obj_root
object* load_objects(object* obj_root, const std::string& file, renderer* ren);

//reloads specified object if it is not empty
bool reload_objects(object* obj, const std::string& file, renderer* ren);

//reload all child object of obj_root
void reload(object* obj_root, renderer* ren);

bool reload_files(object* obj_root, renderer* ren, const std::vector<std::pair<std::string, bool>>& files);

bool changeVisibility_for_files(object* obj_root, renderer* ren, const std::vector<std::pair<std::string, bool>>& files);
