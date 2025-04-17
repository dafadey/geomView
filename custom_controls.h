#pragma once

#include <vector>
#include <string>
#include <memory>

#ifdef MSVC_DYNAMIC_BUILD
  #define MSVC_EXPORT __declspec (dllexport)
#else
  #define MSVC_EXPORT
#endif

struct MSVC_EXPORT geom_view_control {
  std::string name;
  int line = 0;
  std::vector<std::shared_ptr<geom_view_control>> children;
  void add(const std::shared_ptr<geom_view_control>& child);
  void newline();
  virtual void display() = 0;
  void* callback_data = nullptr;

protected:
  geom_view_control() {};
};

struct MSVC_EXPORT geom_view_control_panel : public geom_view_control {
  static std::shared_ptr<geom_view_control_panel> makeCustomPanel(const std::string& name);
  virtual void display();
  
protected:
  geom_view_control_panel() {};
};

struct MSVC_EXPORT geom_view_control_button : public geom_view_control {
  static std::shared_ptr<geom_view_control_button> makeCustomButton(const std::string& name);
  virtual void display();
  void (*callback)(void*) {nullptr};
  
protected:
  geom_view_control_button() {};
};

struct MSVC_EXPORT geom_view_control_slidevalue : public geom_view_control {
  static std::shared_ptr<geom_view_control_slidevalue> makeCustomSlidevalue(const std::string& name, float vmin, float vmax);
  virtual void display();
  void (*callback)(void*) {nullptr};
  float vmin, vmax, value;
  
protected:
  geom_view_control_slidevalue() {};
};

struct MSVC_EXPORT geom_view_control_textLabel : public geom_view_control {
  static std::shared_ptr<geom_view_control_textLabel> makeCustomTextLabel(const std::string& name);
  virtual void display();
  void (*callback)(void*) {nullptr};
  
protected:
  geom_view_control_textLabel() {};
};
