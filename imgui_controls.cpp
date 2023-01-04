#include <string>
#include <iostream>
#include <sstream>
#include <cmath>
#include <filesystem>
#include <algorithm>
#include <vector>

#include <imgui.h>
#include <imgui_internal.h>

#include "imgui_controls.h"
#include "object.h"
#include "draw.h"
#include "tools.h"

static void _update_camera(const vec3f& dir, const vec3f& up, renderer* ren) {
  if(dir*dir) {
    ren->cam_pos = ren->fp_pos + std::sqrt((ren->cam_pos - ren->fp_pos) * (ren->cam_pos - ren->fp_pos)) * dir;
    ren->cam_up = up;
  }
}

namespace ImGui {
  

  bool DoObject(object* obj) {
    if (obj->children.size()) {
      if (ImGui::TreeNode(obj->name.c_str())) {
        if(Button(obj->visible ? "hide" : "show"))
          obj->visible = !obj->visible;
        SameLine();
        if (Button("remove")) {
          ImGui::TreePop();
          return false;
        }

        std::list<object*>::iterator it = obj->children.begin();
        std::vector<std::list<object*>::iterator> to_erase;
        while (it != obj->children.end()) {
          if(!DoObject(*it))
            to_erase.push_back(it);
          it++;
        }
        for(auto& it : to_erase) {
          delete *it;
          obj->children.erase(it);
        }
        ImGui::TreePop();
      }
    } else {
      if(obj->item) {
        PushID(reinterpret_cast<size_t>(obj));
        Checkbox(obj->name.c_str(), &(obj->item->visible));
        PopID();
      }
    }
    return true;
  }

  bool ObjectsControl(object* obj, renderer* ren) {
    ImGui::Begin("objects");
    if (Button("open"))
      OpenPopup("open file");
    if (BeginPopupModal("open file", NULL, ImGuiWindowFlags_None)) {
      static std::filesystem::path path = std::filesystem::current_path();
      static int selected=-1;
      static std::filesystem::path selected_file;
      int items_in_dir=0;

      #ifdef WIN32
      const std::string path_delimiter = "\\";
      #else
      const std::string path_delimiter = "/";
      #endif
      auto dirs = split(path.string(), path_delimiter);
      std::stringstream newpath{};
      #ifndef WIN32
      newpath << '/';
      #endif
      for(int dir_id=0; dir_id<dirs.size(); dir_id++) {
        auto& d = dirs[dir_id];
        if (d.empty())
          continue;
        newpath << d << path_delimiter;
        if(Button(d.c_str())) {
          selected = -1;
          break;
        }
        SameLine();
      }
      //std::cout << "new path is " << newpath.str() << '\n';
      newpath >> path;
      Text("");
      /*
      int item_id=0;
      for(auto const& dir_entry : std::filesystem::directory_iterator(path)) {
        if(Selectable(dir_entry.path().filename().string().c_str(), selected == item_id, ImGuiSelectableFlags_DontClosePopups)) {
          if(std::filesystem::is_directory(dir_entry.path())) {
            selected = -1;
            newpath.clear();
            auto dirs = split(dir_entry.path().string().c_str(), path_delimiter);
            for (auto d : dirs) {
              if (d.empty())
                continue;
              newpath << d << path_delimiter;
            }
            newpath >> path;
          } else
            selected = item_id;
            selected_file = dir_entry.path();
        }
        item_id++;
      }
      */
      std::vector<std::filesystem::path> pathes;
      for (auto const& dir_entry : std::filesystem::directory_iterator(path))
        pathes.push_back(dir_entry.path());
      std::stable_sort(pathes.begin(), pathes.end(), [](const std::filesystem::path& a, const std::filesystem::path& b)->bool {return std::filesystem::is_directory(a) && !std::filesystem::is_directory(b);});

      std::vector<std::string> strings;
      for(const auto& p : pathes)
        strings.push_back(p.filename().string());
      std::vector<const char*> cstrings;
      for (const auto& s : strings)
        cstrings.push_back(s.c_str());
      
      if (ListBox(" ", &selected, cstrings.data(), cstrings.size(), 13)) {
        if (selected != -1) {
          if(std::filesystem::is_directory(pathes[selected])) {
            newpath.clear();
            auto dirs = split(pathes[selected].string().c_str(), path_delimiter);
            for (auto d : dirs) {
              if (d.empty())
                continue;
              newpath << d << path_delimiter;
            }
            newpath >> path;
            selected = -1;
          } else
            selected_file = pathes[selected];
        } 
      }
      
      if (Button("cancel"))
        CloseCurrentPopup();
      SameLine();
      if(selected != -1) {
        if(Button("open")) {
          std::cout << "opening " << selected_file.string() << '\n';
          load_objects(obj, selected_file.string(), ren);
          CloseCurrentPopup();
        }
      }
      //std::cout << "selected = " << selected << '\n';
      EndPopup();
    }
    DoObject(obj);  
    ImGui::End();
    return true;
  }

  bool CameraControl(renderer* ren) {
    bool modified = false;
    vec3f dir{0,0,0};
    vec3f up;
    if(Button("x+")) {
      modified = true;
      dir = vec3f{1,0,0};
      up = vec3f{0,1,0};
    }
    SameLine();
    if(Button("x-")) {
      modified = true;
      dir = vec3f{-1,0,0};
      up = vec3f{0,1,0};
    }
    SameLine();
    if(Button("y+")) {
      modified = true;
      dir = vec3f{0,1,0};
      up = vec3f{1,0,0};
    }
    SameLine();
    if(Button("y-")) {
      modified = true;
      dir = vec3f{0,-1,0};
      up = vec3f{1,0,0};
    }
    SameLine();
    if(Button("z+")) {
      modified = true;
      dir = vec3f{0,0,1};
      up = vec3f{1,0,0};
    }
    SameLine();
    if(Button("z-")) {
      modified = true;
      dir = vec3f{0,0,-1};
      up = vec3f{1,0,0};
    }

    ::_update_camera(dir, up, ren);

    if (Button("center")) {
      modified = true;
      ren->center_camera();
    }
    
    SameLine();

    if (Button("reset")) {
      modified = true;
      ren->reset_camera();
    }

    SameLine();

    modified |= Checkbox("parallel", &(ren->parallel));
    
    return modified;
  }
  
}
