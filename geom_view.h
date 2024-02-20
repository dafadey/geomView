#include <string>
#include <vector>
#include <mutex>

struct object;
struct renderer;

#ifdef MSVC_DYNAMIC_BUILD
  #define MSVC_EXPORT __declspec (dllexport)
#else
  #define MSVC_EXPORT
#endif

struct MSVC_EXPORT geom_view {
  renderer* ren_ptr{nullptr};
  object* obj_root{nullptr};
  void (*controlPointMoved)(void*, std::vector<std::string>& sId, double x, double y, double z) {nullptr};
  void* callbackData {nullptr};
  
private:  
  std::vector<std::string> filenames;
  std::mutex initLock;//, contextLock;
  void init();
  
public:
  static void thread_func(geom_view*);

  void init(const std::string& filename);
  void init(const std::vector<std::string>& filenames);
  void reload();
  void setCallBack(void*, void (*controlPointMoved)(void*, std::vector<std::string>& sId, double x, double y, double z));
};
