#include <fstream>
#include <array>

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
