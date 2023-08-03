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
  while(children.size()) {
    delete *children.begin();
    children.erase(children.begin());
  }
}

int object::memory() {
  int res{};
  if (item)
    res += item->memory();
  res += name.size() * sizeof(char);
  return res;
}

void object::setItemsVisible(bool setting)
{
  for(auto& c : children)
    c->setItemsVisible(setting);
  if(item)
    item->visible = setting;
}

void load_objects(object* obj_root, const std::string& file, renderer* ren) {
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
    if(tokens[0] == "triangles" || tokens[0] == "lines" || tokens[0] == "points") {
      obj = new object();
      type = tokens[0];
      obj->item = newOGLitem(type);
      obj->item->init(ren);
      //ren->items.push_back(obj->item);
      output->children.push_back(obj);
      obj->name = tokens.size() > 1 ? tokens[1] : "<noname>";
      triangles = nullptr;
      lines = nullptr;
      points = nullptr;
      if (type == "triangles")
        triangles = dynamic_cast<OGLtriangles*>(obj->item);
      else if (type == "lines")
        lines = dynamic_cast<OGLlines*>(obj->item);
      else if (type == "points")
        points = dynamic_cast<OGLpoints*>(obj->item);
    }
    else {
      auto vectors = split_vectors(line_no_spaces);
      if(points) {
        if(vectors.size() == 1)
          points->addPoint(make_vector(vectors[0]), 3, vec3f{1.f,1.f,1.f});
        else if (vectors.size() == 2)
          points->addPoint(make_vector(vectors[0]), atof(vectors[1].c_str()), vec3f{ 1.f,1.f,1.f });
        else if (vectors.size() == 3)
          points->addPoint(make_vector(vectors[0]), atof(vectors[1].c_str()), make_vector(vectors[2]));
      }
      if (lines) {
        if(vectors.size() < 2)
          continue;
        std::array<vec3f, 2> coords{make_vector(vectors[0]), make_vector(vectors[1])};
        if (vectors.size() == 2)
          lines->addLine(coords, vec3f{1.f,1.f,1.f});
        else if (vectors.size() == 3)
          lines->addLine(coords, make_vector(vectors[2]));
      }
      if (triangles) {
        if (vectors.size() < 3)
          continue;
        std::array<vec3f, 3> coords{ make_vector(vectors[0]), make_vector(vectors[1]), make_vector(vectors[2]) };
        if (vectors.size() == 3)
          triangles->addTriangle(coords, vec3f{ 1.f,1.f,1.f });
        else if (vectors.size() == 4)
          triangles->addTriangle(coords, make_vector(vectors[3]));
      }
    }
  }
  obj_root->children.push_back(output);
}

static void reload(object* in_obj, std::string& filename, renderer* ren) {
  std::ifstream ini(filename.c_str());
  std::string line;

  std::string type;

  OGLtriangles* triangles{};
  OGLlines* lines{};
  OGLpoints* points{};
  object* obj{};
  auto last_found = in_obj->children.begin();
  std::set<object*> objects_to_remove;
  for(auto o : in_obj->children)
    objects_to_remove.insert(o);
  
  while (std::getline(ini, line)) {
    auto line_no_spaces = remove_chars(remove_chars(remove_chars(remove_chars(line, ' '),'\t'),char(10)),char(13));
    if(line_no_spaces.empty())
      continue;
    auto tokens = split(line_no_spaces, ":");
    if(tokens[0] == "triangles" || tokens[0] == "lines" || tokens[0] == "points") {
      obj = nullptr;
      //super slow search with crasy string comparisons. who cares!
      for(auto it = in_obj->children.begin(); it != in_obj->children.end(); it++) {
        object* o = *it;
        std::string o_type{};
        if(dynamic_cast<OGLtriangles*>(o->item))
          o_type = "triangles";
        if(dynamic_cast<OGLlines*>(o->item))
          o_type = "lines";
        if(dynamic_cast<OGLpoints*>(o->item))
          o_type = "points";
          
        if(tokens.size() > 1 && o->name == tokens[1] && o_type == tokens[0]) { //groups with "unknown" name are skipped
          obj = o;
          last_found = it;
          objects_to_remove.erase(o);
        }
      }
      std::cout << "last_found=" << (*last_found)->name << '\n';
      if(!obj) {
        obj = new object();
        std::string type = tokens[0];
        obj->item = newOGLitem(type);
        obj->item->init(ren);
        obj->name = tokens.size() > 1 ? tokens[1] : "<noname>";
        last_found++;
        in_obj->children.insert(last_found, obj);
      }
      std::cout << "\treloading gorup " << obj->name << '\n';
      triangles = dynamic_cast<OGLtriangles*>(obj->item);
      lines = dynamic_cast<OGLlines*>(obj->item);
      points = dynamic_cast<OGLpoints*>(obj->item);
      if (triangles) {
        std::cout << "\t\treloading triangles\n";
        triangles->clear();
      }
      if (lines) {
        std::cout << "\t\treloading lines\n";
        lines->clear();
      }
      if (points) {
        std::cout << "\t\treloading points\n";
        points->clear();
      }
    }
    else {
      auto vectors = split_vectors(line_no_spaces);
      if(points) {
        if(vectors.size() == 1)
          points->addPoint(make_vector(vectors[0]), 3, vec3f{1.f,1.f,1.f});
        else if (vectors.size() == 2)
          points->addPoint(make_vector(vectors[0]), atof(vectors[1].c_str()), vec3f{ 1.f,1.f,1.f });
        else if (vectors.size() == 3)
          points->addPoint(make_vector(vectors[0]), atof(vectors[1].c_str()), make_vector(vectors[2]));
      }
      if (lines) {
        if(vectors.size() < 2)
          continue;
        std::array<vec3f, 2> coords{make_vector(vectors[0]), make_vector(vectors[1])};
        if (vectors.size() == 2)
          lines->addLine(coords, vec3f{1.f,1.f,1.f});
        else if (vectors.size() == 3)
          lines->addLine(coords, make_vector(vectors[2]));
      }
      if (triangles) {
        if (vectors.size() < 3)
          continue;
        std::array<vec3f, 3> coords{ make_vector(vectors[0]), make_vector(vectors[1]), make_vector(vectors[2]) };
        if (vectors.size() == 3)
          triangles->addTriangle(coords, vec3f{ 1.f,1.f,1.f });
        else if (vectors.size() == 4)
          triangles->addTriangle(coords, make_vector(vectors[3]));
      }
    }
  }
  
  for(auto o : objects_to_remove)
  {
    delete o;
    in_obj->children.remove(o);
  }
}

void reload(object* obj_root, renderer* ren) {
  for(object* c : obj_root->children) {
    std::cout << "reloading file " << c->name << '\n';
    reload(c, c->name, ren);
  }
}
