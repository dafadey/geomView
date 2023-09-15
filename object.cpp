#include <fstream>
#include <array>
#include <set>

#include "object.h"
//#include "saveSTL.h"
#include "draw.h"
#include "timer.h"
#include "OGLitem.h"
#include "tools.h"

object::~object() {
  if(item)
    delete item;
  while(children().size()) {
    delete *children().begin();
    removeChild(*children().begin());
  }
}

void object::addChild(object* c) {
  m_children.push_back(c);
  c->parent = this;
}

std::list<object*>::const_iterator object::addChild(std::list<object*>::const_iterator it, object* c) {
  c->parent = this;
  return m_children.insert(it, c);
}

void object::removeChild(object* c) {
  m_children.remove(c);
}

void object::removeChild(std::list<object*>::const_iterator it) {
  m_children.erase(it);
}

const std::list<object*>& object::children() const {
  return m_children;
}

std::vector<std::string> object::fullName() const {
  std::vector<std::string> s;
  const object* o = this;
  while(o->parent) {
    s.push_back(o->name);
    o=o->parent;
  }
  s.push_back(o->name);
  return s;
}

int object::memory() const {
  int res{};
  if (item)
    res += item->memory();
  res += name.size() * sizeof(char);
  return res;
}

void object::setItemsVisible(bool setting)
{
  for(auto& c : children())
    c->setItemsVisible(setting);
  if(item)
    item->visible = setting;
}

bool load_objects(object* obj_root, const std::string& file, renderer* ren) {
  std::ifstream ini(file.c_str());
  std::string line;

  object* output = new object();
  output->name = file;

  object* obj{nullptr};

  std::string type;

  OGLtriangles* triangles{};
  OGLlines* lines{};
  OGLpoints* points{};
  while (std::getline(ini, line)) {
    auto line_no_spaces = remove_chars(remove_chars(remove_chars(remove_chars(line, ' '),'\t'),char(10)),char(13));
    if(line_no_spaces.empty())
      continue;
    auto tokens = split(line_no_spaces, ":");
    if(tokens[0] == "triangles" || tokens[0] == "lines"  || tokens[0] == "vectors" || tokens[0] == "points"|| tokens[0] == "control_points") {
      obj = new object();
      type = tokens[0];
      obj->item = newOGLitem(type);
      obj->item->init(ren);
      //ren->items.push_back(obj->item);
      output->addChild(obj);
      obj->name = tokens.size() > 1 ? tokens[1] : "<noname>";
      triangles = nullptr;
      lines = nullptr;
      points = nullptr;
      if (type == "triangles")
        triangles = dynamic_cast<OGLtriangles*>(obj->item);
      else if (type == "lines")
        lines = dynamic_cast<OGLlines*>(obj->item);
      else if (type == "vectors")
        lines = dynamic_cast<OGLvectors*>(obj->item);
      else if (type == "points")
        points = dynamic_cast<OGLpoints*>(obj->item);
      else if (type == "control_points")
        points = dynamic_cast<OGLControlPoints*>(obj->item);
    } else {
      auto vects = split_vectors(line_no_spaces);
      if(points) {
        if(vects.size() == 1)
          points->addPoint(make_vector(vects[0]), 3, vec3f{1.f,1.f,1.f});
        else if (vects.size() == 2)
          points->addPoint(make_vector(vects[0]), atof(vects[1].c_str()), vec3f{ 1.f,1.f,1.f });
        else if (vects.size() == 3)
          points->addPoint(make_vector(vects[0]), atof(vects[1].c_str()), make_vector(vects[2]));
      }
      if (lines) {
        if(vects.size() < 2)
          continue;
        std::array<vec3f, 2> coords{make_vector(vects[0]), make_vector(vects[1])};
        if (vects.size() == 2)
          lines->addLine(coords, vec3f{1.f,1.f,1.f});
        else if (vects.size() == 3)
          lines->addLine(coords, make_vector(vects[2]));
      }
      if (triangles) {
        if (vects.size() < 3)
          continue;
        std::array<vec3f, 3> coords{ make_vector(vects[0]), make_vector(vects[1]), make_vector(vects[2]) };
        if (vects.size() == 3)
          triangles->addTriangle(coords, vec3f{ 1.f,1.f,1.f });
        else if (vects.size() == 4)
          triangles->addTriangle(coords, make_vector(vects[3]));
      }
    }
  }
  if(output->children().size()) {
    obj_root->addChild(output);
    return true;
  } else {
    delete output;
    return false;
  }
}

static void reload(object* in_obj, std::string& filename, renderer* ren) {
  std::ifstream ini(filename.c_str());
  std::string line;

  std::string type;

  OGLtriangles* triangles{};
  OGLlines* lines{};
  OGLpoints* points{};
  object* obj{};
  auto last_found = in_obj->children().begin();
  std::set<object*> objects_to_remove;
  for(auto o : in_obj->children())
    objects_to_remove.insert(o);
  
  while (std::getline(ini, line)) {
    auto line_no_spaces = remove_chars(remove_chars(remove_chars(remove_chars(line, ' '),'\t'),char(10)),char(13));
    if(line_no_spaces.empty())
      continue;
    auto tokens = split(line_no_spaces, ":");
    if(tokens[0] == "triangles" || tokens[0] == "lines" || tokens[0] == "vectors" || tokens[0] == "points" || tokens[0] == "control_points") {
      std::cout << "working with " << tokens[1] << '\n';
      obj = nullptr;
      //super slow search with crazy string comparisons. who cares!
      for(auto it = in_obj->children().begin(); it != in_obj->children().end(); it++) {
        object* o = *it;
        std::string o_type{};
        if(dynamic_cast<OGLtriangles*>(o->item))
          o_type = "triangles";
        if(dynamic_cast<OGLlines*>(o->item))
          o_type = "lines";
        if(dynamic_cast<OGLvectors*>(o->item))
          o_type = "vectors";
        if(dynamic_cast<OGLpoints*>(o->item))
          o_type = "points";
        if(dynamic_cast<OGLControlPoints*>(o->item))
          o_type = "control_points";
          
        if(tokens.size() > 1 && o->name == tokens[1] && o_type == tokens[0]) { //groups with "unknown" name are skipped
          obj = o;
          last_found = it;
          objects_to_remove.erase(o);
        }
      }
      if(obj)
        std::cout << "found!\n";
      std::cout << "last_found=" << (*last_found)->name << '\n';
      if(!obj) {
        obj = new object();
        std::string type = tokens[0];
        obj->item = newOGLitem(type);
        obj->item->init(ren);
        obj->name = tokens.size() > 1 ? tokens[1] : "<noname>";
        last_found++;
        last_found = in_obj->addChild(last_found, obj);
      }
      std::cout << "\treloading gorup " << obj->name << '\n';
      triangles = dynamic_cast<OGLtriangles*>(obj->item);
      lines = dynamic_cast<OGLlines*>(obj->item);
      points = dynamic_cast<OGLpoints*>(obj->item);
      obj->item->clear();
    }
    else {
      auto vects = split_vectors(line_no_spaces);
      if(points) {
        if(vects.size() == 1)
          points->addPoint(make_vector(vects[0]), 3, vec3f{1.f,1.f,1.f});
        else if (vects.size() == 2)
          points->addPoint(make_vector(vects[0]), atof(vects[1].c_str()), vec3f{ 1.f,1.f,1.f });
        else if (vects.size() == 3)
          points->addPoint(make_vector(vects[0]), atof(vects[1].c_str()), make_vector(vects[2]));
      }
      if (lines) {
        if(vects.size() < 2)
          continue;
        std::array<vec3f, 2> coords{make_vector(vects[0]), make_vector(vects[1])};
        if (vects.size() == 2)
          lines->addLine(coords, vec3f{1.f,1.f,1.f});
        else if (vects.size() == 3)
          lines->addLine(coords, make_vector(vects[2]));
      }
      if (triangles) {
        if (vects.size() < 3)
          continue;
        std::array<vec3f, 3> coords{ make_vector(vects[0]), make_vector(vects[1]), make_vector(vects[2]) };
        if (vects.size() == 3)
          triangles->addTriangle(coords, vec3f{ 1.f,1.f,1.f });
        else if (vects.size() == 4)
          triangles->addTriangle(coords, make_vector(vects[3]));
      }
    }
  }
  
  for(auto o : objects_to_remove)
  {
    delete o;
    in_obj->removeChild(o);
  }
}

void reload(object* obj_root, renderer* ren) {
  for(object* c : obj_root->children()) {
    std::cout << "reloading file " << c->name << '\n';
    reload(c, c->name, ren);
  }
}
