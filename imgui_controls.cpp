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
#include "interface.h"

extern mainwin_config mainwin_conf;

static void _update_camera(const vec3f& dir, const vec3f& up, renderer* ren) {
  if(dir*dir) {
    ren->cam_pos = ren->fp_pos + std::sqrt((ren->cam_pos - ren->fp_pos) * (ren->cam_pos - ren->fp_pos)) * dir;
    ren->cam_up = up;
  }
}

static std::filesystem::path path = std::filesystem::current_path();

#ifndef MAXRECENTS
#define MAXRECENT 13
#endif

void addRecentFile(const std::string& file)
{
  for(const auto& fl : mainwin_conf.recent_files)
    if(file == fl)
      return;
  if(mainwin_conf.recent_files.size() < MAXRECENT)
    mainwin_conf.recent_files.push_back(file);
  else {
    int i=1;
    for(;i<mainwin_conf.recent_files.size();i++)
      mainwin_conf.recent_files[i-1] = mainwin_conf.recent_files[i];
    mainwin_conf.recent_files[i] = file;
  }
}

namespace ImGui {

  bool DoObject(object* obj) {
    if (obj->children().size()) {
      if (ImGui::TreeNode(obj->name.c_str())) {
        if(Button(obj->visible ? "untick" : "tick"))
          obj->setItemsVisible(!obj->visible);
        SameLine();
        if(Button(obj->visible ? "hide" : "show"))
          obj->visible = !obj->visible;
        SameLine();
        if (Button("remove")) {
          ImGui::TreePop();
          return false;
        }

        std::list<object*>::const_iterator it = obj->children().begin();
        std::vector<std::list<object*>::const_iterator> to_erase;
        while (it != obj->children().end()) {
          if(!DoObject(*it))
            to_erase.push_back(it);
          it++;
        }
        for(auto& it : to_erase) {
          delete *it;
          obj->removeChild(it);
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
    SameLine();
    if (mainwin_conf.recent_files.size() && Button("recent"))
      OpenPopup("open recent");
    SameLine();
    if(Button("reload"))
      reload(obj, ren);
    if (BeginPopupModal("open file", NULL, ImGuiWindowFlags_None)) {
      static int selected=-1;
      //static std::filesystem::path selected_file;
      static std::string selected_file;
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
      newpath >> path;
      Text("");
      std::vector<std::pair<std::string, bool>> pathes;
      for (auto const& dir_entry : std::filesystem::directory_iterator(path))
        pathes.push_back(std::make_pair(dir_entry.path().filename().string(), std::filesystem::is_directory(dir_entry)));

      std::stable_sort(pathes.begin(), pathes.end(), [](const std::pair<std::string, bool>& a, const std::pair<std::string, bool>& b)->bool {
        if(a.second == b.second)
          return a.first < b.first;
        return a.second && !b.second;
      });

      std::vector<const char*> cstrings;
      for (const auto& p : pathes)
        cstrings.push_back(p.first.c_str());
      
      if (ListBox(" ", &selected, cstrings.data(), cstrings.size(), 22)) {
        if (selected != -1) {
          if(pathes[selected].second) {
            newpath.clear();
            auto dirs = split((path.string() + pathes[selected].first).c_str(), path_delimiter);
            for (auto d : dirs) {
              if (d.empty())
                continue;
              newpath << d << path_delimiter;
            }
            newpath >> path;
            selected = -1;
          } else
            selected_file = path.string()+pathes[selected].first;
        } 
      }
      
      if (Button("cancel"))
        CloseCurrentPopup();
      SameLine();
      if(selected != -1) {
        if(Button("open")) {
          std::cout << "opening " << selected_file << '\n';
          if(load_objects(obj, selected_file, ren))
            addRecentFile(selected_file);
          CloseCurrentPopup();
        }
      }
      EndPopup();
    }

    if (BeginPopupModal("open recent", NULL, ImGuiWindowFlags_None)) {
      for(int i=mainwin_conf.recent_files.size()-1; i>=0; i--) {
        const auto& fn = mainwin_conf.recent_files[i];
        if(Button(fn.c_str())) {
          std::cout << "opening recent " << fn << '\n';
          load_objects(obj, fn, ren);
          CloseCurrentPopup();
        }
      }
      if (Button("clear recent")) {
        mainwin_conf.recent_files.clear();
        CloseCurrentPopup();
      }
      if (Button("close"))
        CloseCurrentPopup();
      EndPopup();
    }
    DoObject(obj);  
    ImGui::End();
    return true;
  }

  bool CameraControl(renderer* ren) {
    bool modified = false;
    vec3f dir = ren->fp_pos - ren->cam_pos;
    vec3f up = ren->cam_up;
    normalize(dir);
    normalize(up);
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
    SameLine();
    if(Button("+90")) {
      modified = true;
      up = cross_prod(dir,up);
    }
    SameLine();
    if(Button("-90")) {
      modified = true;
      up = cross_prod(up,dir);
    }
    
    if(modified)
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

    ColorEdit3("bg color", ren->bg_color.data());
    
    return modified;
  }
  
}
