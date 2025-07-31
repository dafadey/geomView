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
  typedef std::pair<void(*)(void*), void*> postponed_callback_type;
  typedef std::vector<postponed_callback_type> postponed_callbacks_type;
  //callback are postponed after draw command is done
  //this helps to modify geom_view_controls on fly (inside callback)
  //i.e. you can dynamically chage children of your controls inside callback to modify you UI
  std::string name;
  int line = 0;
  std::vector<std::shared_ptr<geom_view_control>> children;
  void add(const std::shared_ptr<geom_view_control>& child);
  void clear();
  bool remove(const std::shared_ptr<geom_view_control>& child);
  void newline();
  virtual void display(postponed_callbacks_type&) = 0;
  void* callback_data = nullptr;
  bool visible{true};
protected:
  geom_view_control() {};
};

struct MSVC_EXPORT geom_view_control_panel : public geom_view_control {
  static std::shared_ptr<geom_view_control_panel> makeCustomPanel(const std::string& name);
  virtual void display(postponed_callbacks_type&);
  
protected:
  geom_view_control_panel() {};
};

struct MSVC_EXPORT geom_view_control_button : public geom_view_control {
  static std::shared_ptr<geom_view_control_button> makeCustomButton(const std::string& name);
  virtual void display(postponed_callbacks_type&);
  void (*callback)(void*) {nullptr};
  
protected:
  geom_view_control_button() {};
};

struct MSVC_EXPORT geom_view_control_slidevalue : public geom_view_control {
  static std::shared_ptr<geom_view_control_slidevalue> makeCustomSlidevalue(const std::string& name, float vmin, float vmax);
  virtual void display(postponed_callbacks_type&);
  void (*callback)(void*) {nullptr};
  float vmin, vmax, value;
  
protected:
  geom_view_control_slidevalue() {};
};

struct MSVC_EXPORT geom_view_control_inputInt : public geom_view_control {
  static std::shared_ptr<geom_view_control_inputInt> makeCustomInputInt(const std::string& name);
  virtual void display(postponed_callbacks_type&);
  void (*callback)(void*) {nullptr};
  int value{};
  
protected:
  geom_view_control_inputInt() {};
};

struct MSVC_EXPORT geom_view_control_textLabel : public geom_view_control {
  static std::shared_ptr<geom_view_control_textLabel> makeCustomTextLabel(const std::string& name);
  virtual void display(postponed_callbacks_type&);
  void (*callback)(void*) {nullptr};
  
protected:
  geom_view_control_textLabel() {};
};

struct MSVC_EXPORT geom_view_control_list : public geom_view_control {
  struct item {
    void* ptr; // for pick callback
    std::vector<std::string> props; // display properties
    bool selected{};
  };
  
  static std::shared_ptr<geom_view_control_list> makeCustomList();
  virtual void display(postponed_callbacks_type&);
  std::vector<std::string> header;
  std::vector<item> items; // removes all previous content with new one
  void (*callback)(void*) {nullptr}; //called when something is changed
  
protected:
  geom_view_control_list() {};
};

struct MSVC_EXPORT geom_view_control_radio : public geom_view_control {
  static std::shared_ptr<geom_view_control_radio> makeCustomRadio(const std::vector<std::string>& names);
  virtual void display(postponed_callbacks_type&);
  std::vector<std::string> names;
  int choice{};
  void (*callback)(void*) {nullptr};
  
protected:
  geom_view_control_radio() {};
};

struct MSVC_EXPORT geom_view_control_checkbox : public geom_view_control {
  static std::shared_ptr<geom_view_control_checkbox> makeCustomCheckBox(const std::string& name);
  virtual void display(postponed_callbacks_type&);
  bool checked{};
  void (*callback)(void*) {nullptr};
  
protected:
  geom_view_control_checkbox() {};
};
