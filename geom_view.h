#include <string>
#include <vector>

struct object;
struct renderer;

struct geom_view {
  renderer* ren_ptr{nullptr};
  object* obj_root{nullptr};
  void (*controlPointMoved)(void*, std::vector<std::string>& sId, double x, double y, double z) {nullptr};
  void* callbackData {nullptr};
  
  void init(const std::string& filename);
  void reload();
  void setCallBack(void*, void (*controlPointMoved)(void*, std::vector<std::string>& sId, double x, double y, double z));
};

