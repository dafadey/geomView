#ifdef _MSC_VER
#define NOMINMAX
#endif
#include <string>
#include <iostream>
#include <sstream>
#include <cmath>
#if NOSTDFILESYSTEM
#define FSNAMESPACE STD11FS // use our own win API (hope the old std is not the case for g++ builds) based filesystem implementation (only needed features ofcourse)
#include "filesystem_internal.h"
#else
#define FSNAMESPACE std // std::filesystem will be used
#include <filesystem>
#endif

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

static FSNAMESPACE::filesystem::path path = FSNAMESPACE::filesystem::current_path();

std::string SFW(const std::wstring& in) {
  char tmp[113];
  std::wcstombs(tmp, in.c_str(), 111);
  return std::string(tmp);
}

#ifndef MAXRECENTS
#define MAXRECENT 13
#endif

void addRecentFile(const std::string& file)
{
  for(const auto& fl : mainwin_conf.recent_files) {
    if(file == fl)
      return;
  }
  if(mainwin_conf.recent_files.size() < MAXRECENT)
    mainwin_conf.recent_files.push_back(file);
  else {
    int i=1;
    for(;i<mainwin_conf.recent_files.size();i++)
      mainwin_conf.recent_files[i-1] = mainwin_conf.recent_files[i];
    mainwin_conf.recent_files[i-1] = file;
  }
}

bool match(const std::string& s, const std::vector<std::string>& filter) {
  //*a?b
  //sdkasdflnbsd
  bool total_res = false;  
  for(const auto& f : filter) {
    if(f.empty())
      continue;
    int i=0;
    int j=0;
    int j0 = f[0] == '*' ? 0 : -1;
    bool result=false;
    while(true) {
      if(s[i] == f[j]) {
        i++;
        j++;
        if(j==f.size() && i!=s.size())
          j = std::min(j0 + 1, (int)f.size() - 1);
      } else {
        if((j0>=0 && f[j0] == '*' && j==j0+1) || f[j] == '*') {
          i++;
        } else if(f[j] == '?') {
          i++;
          j++;
        }
        else {
          if (j0 != -1)
            j = std::min(j0 + 1, (int)f.size() - 1);
          else {
            result = false;
            break;
          }
        }
      }
      if(j==0 && i==0 && f[0] != '*') {
        result = false;
        break;
      }
      if(j == f.size()) {
        result = (i == s.size() || f[j - 1] == '*');
        break;
      }
      if(i == s.size()) {
        result = false;
        break;
      }

      if(f[j] == '*') {
        j0=j;
        j++;
        if(j == f.size()) {
          result = true;
          break;
        }
      }
    }
    total_res |= result;
  }
  return total_res;
}

bool searchByFirstLettersNoCase(const std::string& s, const std::string& highlightMask) {
  for(int i=0; i<std::min(s.size(), highlightMask.size()); i++) {
    if(s[i] != highlightMask[i] && s[i] != highlightMask[i] + 32) {
      return false;
    }
  }
  return true;
}

namespace ImGui {

  void FilterTickObject(object* obj, const std::vector<std::string>& filter, bool tick) {
    for(auto ch : obj->children())
      FilterTickObject(ch, filter, tick);
    if(obj->item) {
      std::string fn;
      std::vector<std::string> fullNameItemsReverse = obj->fullName();
      for(int i=fullNameItemsReverse.size()-1; i>=0; i--)
        fn += fullNameItemsReverse[i];

      if(match(fn, filter))
        obj->item->visible = tick;
    }
  }

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
      static std::string highlightMask="";
      static std::wstring selected_file;
      int items_in_dir=0;
      
      #ifdef WIN32
      const std::wstring path_delimiter = L"\\";
      #else
      const std::wstring path_delimiter = L"/";
      #endif
      auto dirs = split(path.wstring(), path_delimiter);
      std::wstringstream newpath{};
      #ifndef WIN32
      newpath << '/';
      #endif
      bool pressed = false;
      for(int dir_id=0; dir_id<dirs.size(); dir_id++) {
        auto& d = dirs[dir_id];
        if (d.empty())
          continue;
        newpath << d << path_delimiter;
        if(Button(SFW(d).c_str())) {
          selected = -1;
          pressed = true;
          break;
        }
        if(dir_id + 2 == dirs.size())
          continue;
        SameLine();
      }
      path = newpath.str();
      static char filter[128]="*";
      bool listenToFirstLetters = true;
      if(InputText("filter", filter, IM_ARRAYSIZE(filter)))
        listenToFirstLetters = false;
      Text(!highlightMask.empty() ? highlightMask.c_str() : " ");
      std::vector<std::string> filters = split(std::string(filter), std::string(";"));
      
      std::vector<std::pair<std::wstring, bool>> pathes;
      
      for (auto const& dir_entry : FSNAMESPACE::filesystem::directory_iterator(path))
        pathes.push_back(std::make_pair(dir_entry.path().filename().wstring(), FSNAMESPACE::filesystem::is_directory(dir_entry)));

      std::stable_sort(pathes.begin(), pathes.end(), [](const std::pair<std::wstring, bool>& a, const std::pair<std::wstring, bool>& b)->bool {
                                                        if(a.second == b.second)
                                                          return SFW(a.first) < SFW(b.first); // inefficient but valgrind complains on that otherwise (most likelly false positive but annoying anyways)
                                                        return a.second && !b.second;
                                                      });

      std::vector<std::string> strings;
      std::vector<int> selectedLookUp;
      for (int i=0; i<pathes.size(); i++) {
        std::string item = SFW(pathes[i].first);
        if(match(item,filters) || pathes[i].second) {
          strings.push_back(SFW(pathes[i].first));
          selectedLookUp.push_back(i);
        }
      }
      
      std::vector<const char*> cstrings;
      for (const auto& p : strings)
        cstrings.push_back(p.c_str());
      
      if (ListBox(" ", &selected, cstrings.data(), cstrings.size(), 22)) {
        if (selected != -1) {
          const auto& p=pathes[selectedLookUp[selected]];
          if(p.second) {
            newpath.str(L"");
            auto dirs = split(path.wstring() + p.first, path_delimiter);
            for (auto d : dirs) {
              if (d.empty())
                continue;
              newpath << d << path_delimiter;
            }
            path = newpath.str();
            selected = -1;
          }
        } 
        highlightMask = "";
      }
      
      ImGuiIO& gio = ImGui::GetIO();
      ImGuiContext& g = *ImGui::GetCurrentContext();
      ImGuiWindow* currentWindow = g.CurrentWindow; //open popup
      ImGuiWindow* listWindow = nullptr;
      
      for(auto win : g.Windows) {
        if(win->ParentWindow == currentWindow) {
          listWindow = win;
          break;
        }
      }
      
      if(listWindow) {
        for (int key = 0; key < IM_ARRAYSIZE(gio.KeysDown); key++) {
          if (ImGui::IsKeyPressed(key))
          {
            if(key == ImGui::GetKeyIndex(ImGuiKey_::ImGuiKey_DownArrow)) {
              selected = selected < (int) cstrings.size() - 1 ? selected + 1 : (int) cstrings.size() - 1;
              listWindow->Scroll.y += g.Style.ItemSpacing.y + g.FontSize;
              highlightMask = "";
            }
            if(key == ImGui::GetKeyIndex(ImGuiKey_::ImGuiKey_UpArrow)) {
              selected = selected > 0 ? selected-1 : selected;
              listWindow->Scroll.y -= g.Style.ItemSpacing.y + g.FontSize;
              highlightMask = "";
            }
            if(key <= 90 && key>=65 && listenToFirstLetters) {
              highlightMask += char(key);
              int itemId=-1;
              for(int i=0; i<strings.size(); i++) {
                if(searchByFirstLettersNoCase(strings[i], highlightMask)) {
                  itemId=i;
                  break;
                }
              }
              if(itemId!=-1) {
                selected = itemId;
                listWindow->Scroll.y = selected * (g.Style.ItemSpacing.y + g.FontSize);
              }
            }
          }
        }
      }
      
      if(selected != -1 && selected < selectedLookUp.size())
        selected_file = path.wstring() + pathes[selectedLookUp[selected]].first;
      
      if (Button("cancel"))
        CloseCurrentPopup();
      SameLine();
      if(selected != -1) {
        if(Button("open")) {
          std::cout << "opening " << SFW(selected_file) << '\n';
          if(load_objects(obj, SFW(selected_file), ren))
            addRecentFile(SFW(selected_file));
          CloseCurrentPopup();
          if(obj->children().size() == 1)
            ren->reset_camera();
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
    
    static char tickfilter[128]="*";
    if(InputText("aaa",tickfilter, IM_ARRAYSIZE(tickfilter)))
      std::cout << "tickuntick filter activted\n";
    int doFilterTick=0;
    SameLine();
    if(Button("tick"))
      doFilterTick=1;
    SameLine();
    if(Button("untick"))
      doFilterTick=2;
    
    if(doFilterTick)
      FilterTickObject(obj, split(std::string(tickfilter), std::string(";")), doFilterTick==1 ? true : false);
    
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
