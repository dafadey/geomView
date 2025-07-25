#include <iostream>
#include "geom_view.h"

void moved(void* raw);

void selection_callback(void* raw, const std::vector<std::tuple<std::vector<std::string>, size_t, float>>& input);

struct interface {
  geom_view gv;
  std::shared_ptr<geom_view_control_panel> panel;
  std::shared_ptr<geom_view_control_button> button1;
  std::shared_ptr<geom_view_control_button> button2;
  std::shared_ptr<geom_view_control_slidevalue> fslider;
  std::shared_ptr<geom_view_control_textLabel> text_label;
  
  bool select=true;
  
  void init(int argc, char* argv[]) {
    gv.init();
  
    if(argc==2) {
      std::vector<std::pair<std::string, bool>> files;
      files.push_back(std::pair<std::string, bool>(argv[1], true));
      gv.reload(files, true);
    }
    
    panel = geom_view_control_panel::makeCustomPanel("myPanel");
    
    gv.addCustomControl(std::static_pointer_cast<geom_view_control>(panel));
    
    button1 = geom_view_control_button::makeCustomButton("select");
    panel->add(std::static_pointer_cast<geom_view_control>(button1));
    button1->callback = [] (void* raw) {
                          interface* iface = reinterpret_cast<interface*>(raw);
                          iface->text_label->name = "RMB to select";
                          iface->select = true;};
    button1->callback_data = this;

    button2 = geom_view_control_button::makeCustomButton("deselect");
    panel->add(std::static_pointer_cast<geom_view_control>(button2));
    button2->callback = [] (void* raw) {
                          interface* iface = reinterpret_cast<interface*>(raw);
                          iface->text_label->name = "RMB to deselect";
                          iface->select = false;};
    button2->callback_data = this;

    fslider = geom_view_control_slidevalue::makeCustomSlidevalue("value", -.1, .1);
    fslider->newline();
    fslider->vmax = 3.3;
    panel->add(std::static_pointer_cast<geom_view_control>(fslider));
    fslider->callback = moved;
    fslider->callback_data = this;

    text_label = geom_view_control_textLabel::makeCustomTextLabel("none");
    text_label->newline();
    text_label->newline();
    text_label->callback_data = this;
    panel->add(std::static_pointer_cast<geom_view_control>(text_label));
    
    gv.setSelectCallBack(this, selection_callback);
  }
};

void moved(void* raw) {
  interface* iface = reinterpret_cast<interface*>(raw);
  iface->text_label->name = "slider is moved";
}

void selection_callback(void* raw, const std::vector<std::tuple<std::vector<std::string>, size_t, float>>& input) {
  interface* iface = reinterpret_cast<interface*>(raw);
  if(input.size()) {
    iface->text_label->name = std::get<0>(input[0])[1] +":"+ std::get<0>(input[0])[0] + ":" + std::to_string(std::get<1>(input[0]));
    std::string name;
    for(int i=std::get<0>(input[0]).size()-1; i >=0 ; i--)
      name += std::get<0>(input[0])[i] + ':';
    name += std::to_string(std::get<1>(input[0]));
    std::cout << "highlighting \"" << name << "\"\n";
    iface->gv.highlight(name, iface->select);
  }
}


int main(int argc, char* argv[]) {

  interface iface;
  iface.init(argc, argv);
  
  std::string cmd;
  while(cmd != "exit") {
    std::cin >> cmd;
    if(cmd == "exit")
      break;
    else {
      std::cout << "unknown command\n";
      continue;
    }
  }
  return 0;
}
