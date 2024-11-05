#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

#ifdef WIN32
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
  renderer* ren_ptr{nullptr};
  object* obj_root{nullptr};
  imgui_interface* iface;
  void (*controlPointMoved)(void*, std::vector<std::string>& sId, double x, double y, double z) {nullptr};
  void* callbackData {nullptr};

private:
  std::vector<std::string> filenames;
  std::mutex windowCreationLock;
  std::condition_variable windowCreationCV;
  std::mutex reloadLock;
  bool reloadFlag {false};
  void init();
  
public:
  geom_view();
  ~geom_view();
	
  static void thread_func(geom_view*);

#ifdef WIN32
  HWND getNativeWin32Handler();
  void setParentWin32Handler(HWND parentMSWindowHandler);
#endif

  void init(const std::string& filename);
  void init(const std::vector<std::string>& filenames);
  void reload();
  void setCallBack(void*, void (*controlPointMoved)(void*, std::vector<std::string>& sId, double x, double y, double z));
};
