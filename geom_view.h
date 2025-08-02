#pragma once

#include <string>
#include <array>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "custom_controls.h"

#ifdef _WIN32
#include <windows.h>
#endif

struct object;
struct renderer;
struct imgui_interface;
struct fb2way;

#ifdef MSVC_DYNAMIC_BUILD
  #define MSVC_EXPORT __declspec (dllexport)
#else
  #define MSVC_EXPORT
#endif

struct MSVC_EXPORT geom_view {
  
  const static int LEFT_BUTTON;
  const static int MIDDLE_BUTTON;
  const static int RIGHT_BUTTON;
  
  struct ViewControls {
    int rotateButton;
    int panButton;
    int selectButton;
  };
  
  ViewControls viewControls;
  
  struct UIappearance {
    bool imgui_cam_control{true};
    bool imgui_object_control{true};
    bool native_cam_control{true};
  };
  
  renderer* ren_ptr{nullptr};
  object* obj_root{nullptr};
  imgui_interface* iface{nullptr};
  fb2way* fb2{nullptr};
  //void (*controlPointMoved)(void*, std::vector<std::string>& sId, double x, double y, double z) {nullptr};
  //void* callbackData {nullptr};
  bool runInThread{true}; //by default library runs in thread because ideologically you add library to visualize something on fly, if you want to create singlethreaded app based on this library set to false
  std::array<int, 2> screenGeo{0, 0};

private:
  std::vector<std::pair<std::string, bool>> files; //this is used to pass data to another thread and protected with reloadLock
  bool resetCameraRequest{false}; // this is used to request for reset camera after reload 
  std::mutex windowCreationLock;
  std::condition_variable windowCreationCV;
  std::mutex reloadLock;
  bool reloadFlag {false};
  bool changeVisibilityFlag {false};
  bool offscreen{}; // does not create a window, renders to buffer
  
public:
  //switches to offscreen mode
  void setOffsсreen(int nx, int ny);
  bool isOffscreen();
  
  // retreives image buffer without controls and selection highlight
  // output format:
  //  pixel format is 3 chars (RGB)
  //  pixels are in row order
  void getBuffer(int& nx, int& ny, std::vector<unsigned char>& buff);
  void makeShot();
  
  geom_view();
  ~geom_view();

  UIappearance appearance{};
  
  static void thread_func(geom_view*);
  std::thread::id th_id;

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

  void showOrigin(bool);

  void highlight(const std::vector<std::string>&, bool = true); // vector is an object in format {root:name:name:name:id} if id=-1 all items in group

  void highlight(const std::string&, bool = true); // string is an object in format root:name:name:name:id if id=-1 all items in group
  
  void addCustomControl(const std::shared_ptr<geom_view_control>& cc);
  
  static std::vector<std::string> tokenize(const std::string& in, const std::string& delim);
};
