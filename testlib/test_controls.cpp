#include <iostream>
#include "geom_view.h"

void moved(void* raw);

void selection_callback(void* raw, const std::vector<std::tuple<std::vector<std::string>, size_t, float>>& input);

std::vector<std::string> string_to_vector(const std::string& in) {
  std::vector<std::string> res;
  std::string current;
  for(int i=0;i<in.size();i++) {
    if(in.c_str()[i] == ':') {
      res.push_back(current);
      current = "";
    } else
      current += in.c_str()[i];
  }
  if(current.size())
    res.push_back(current);
  return res;
}

struct interface {
  geom_view gv;
  std::shared_ptr<geom_view_control_panel> panel;
  std::shared_ptr<geom_view_control_button> button1, button2, button3, button11, button21;
  std::shared_ptr<geom_view_control_textinput> text_input;
  std::shared_ptr<geom_view_control_slidevalue> fslider;
  std::shared_ptr<geom_view_control_textLabel> text_label;
  
  int select=0;
  
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
                          iface->select = 1;};
    button1->callback_data = this;

    button2 = geom_view_control_button::makeCustomButton("deselect");
    panel->add(std::static_pointer_cast<geom_view_control>(button2));
    button2->callback = [] (void* raw) {
                          interface* iface = reinterpret_cast<interface*>(raw);
                          iface->text_label->name = "RMB to deselect";
                          iface->select = -1;};
    button2->callback_data = this;

    button3 = geom_view_control_button::makeCustomButton("none");
    panel->add(std::static_pointer_cast<geom_view_control>(button3));
    button3->callback = [] (void* raw) {
                          interface* iface = reinterpret_cast<interface*>(raw);
                          iface->text_label->name = "RMB does nothing";
                          iface->select = 0;};
    button3->callback_data = this;

    text_input = geom_view_control_textinput::makeCustomTextInput("item to select");
    text_input->newline();
    panel->add(std::static_pointer_cast<geom_view_control>(text_input));
    
    button11 = geom_view_control_button::makeCustomButton("select");
    panel->add(std::static_pointer_cast<geom_view_control>(button11));
    button11->callback = [] (void* raw) {
                          interface* iface = reinterpret_cast<interface*>(raw);
                          iface->gv.highlight(string_to_vector(iface->text_input->text.data()), true);
                          };
    button11->callback_data = this;

    button21 = geom_view_control_button::makeCustomButton("deselect");
    panel->add(std::static_pointer_cast<geom_view_control>(button21));
    button21->callback = [] (void* raw) {
                          interface* iface = reinterpret_cast<interface*>(raw);
                          iface->gv.highlight(string_to_vector(iface->text_input->text.data()), false);
                          };
    button21->callback_data = this;
    
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
  if(iface->select == 0)
    return;
  if(input.size()) {
    iface->text_label->name = std::get<0>(input[0])[1] +":"+ std::get<0>(input[0])[0] + ":" + std::to_string(std::get<1>(input[0]));
    std::string name;
    for(int i=std::get<0>(input[0]).size()-1; i >=0 ; i--)
      name += std::get<0>(input[0])[i] + ':';
    name += std::to_string(std::get<1>(input[0]));
    std::cout << "highlighting \"" << name << "\"\n";
    iface->gv.highlight(name, iface->select == 1 ? true : false);
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
