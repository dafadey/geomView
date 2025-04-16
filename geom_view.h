#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "custom_controls.h"

#ifdef _WIN32
#include <windows.h>
#endif

struct object;
struct renderer;
struct imgui_interface;

#ifdef MSVC_DYNAMIC_BUILD
  #define MSVC_EXPORT __declspec (dllexport)
#else
  #define MSVC_EXPORT
#endif

struct MSVC_EXPORT geom_view {
  struct UIappearance {
    bool imgui_cam_control{true};
    bool imgui_object_control{true};
    bool native_cam_control{true};
  };
  
  renderer* ren_ptr{nullptr};
  object* obj_root{nullptr};
  imgui_interface* iface{nullptr};
  //void (*controlPointMoved)(void*, std::vector<std::string>& sId, double x, double y, double z) {nullptr};
  //void* callbackData {nullptr};

private:
  std::vector<std::pair<std::string, bool>> files; //this is used to pass data to another thread and protected with reloadLock
  bool resetCameraRequest{false}; // this is used to request for reset camera after reload 
  std::mutex windowCreationLock;
  std::condition_variable windowCreationCV;
  std::mutex reloadLock;
  bool reloadFlag {false};
  bool changeVisibilityFlag {false};
  
public:
  geom_view();
  ~geom_view();

  UIappearance appearance{};
  
  static void thread_func(geom_view*);

#ifdef _WIN32
  HWND getNativeWin32Handler();
  void setParentWin32Handler(HWND parentMSWindowHandler);
#endif

  void close();
  void init();
  void init(const std::string& filename);
  void init(const std::vector<std::string>& filenames);

  //reloads all
  void reload(bool resetCamera = false);

  //keeps all mentioned if flag is false
  //deletes all previously loaded but not mentioned
  //reloads specified if flag is true
  void reload(const std::vector<std::pair<std::string, bool>>& files, bool resetCamera = false);
  
  //sets visibility for listed
  //does not modify those not listed
  void visibilities(const std::vector<std::pair<std::string, bool>>& vis);
  
  void setCallBack(void*, void (*controlPointMoved)(void*, std::vector<std::string>& sId, double x, double y, double z));

  void setSelectCallBack(void*, void (*selected)(void*, const std::vector<std::tuple<std::vector<std::string>, size_t, float>>&));

  void centerCamera();
  
  void resetCamera();
  
  void addCustomControl(const std::shared_ptr<geom_view_control>& cc);
  
  static std::vector<std::string> tokenize(const std::string& in, const std::string& delim);
};
