#include <iostream>
#include "geom_view.h"


void pushed(void*) {
  std::cout << "button is pushed\n";
}

void moved(void*) {
  std::cout << "slider is moved\n";
}

void selection_callback(void* raw, const std::vector<std::tuple<std::vector<std::string>, size_t, float>>& input) {
  std::shared_ptr<geom_view_control_textLabel> data = *(reinterpret_cast<std::shared_ptr<geom_view_control_textLabel>*>(raw));
  if(input.size()) {
    data->name = std::get<0>(input.back())[0];
  }
}

int main() {
  geom_view gv;
  gv.init();
  
  auto panel = geom_view_control_panel::makeCustomPanel("myPanel");
  
  gv.addCustomControl(std::static_pointer_cast<geom_view_control>(panel));
  
  auto button = geom_view_control_button::makeCustomButton("myButton");
  panel->add(std::static_pointer_cast<geom_view_control>(button));
  button->callback = pushed;


  auto fslider = geom_view_control_slidevalue::makeCustomSlidevalue("value", -.1, .1);
  fslider->newline();
  fslider->vmax = 3.3;
  panel->add(std::static_pointer_cast<geom_view_control>(fslider));
  fslider->callback = moved;


  auto text_label = geom_view_control_textLabel::makeCustomTextLabel("none");
  text_label->newline();
  text_label->newline();
  panel->add(std::static_pointer_cast<geom_view_control>(text_label));
  
  gv.setSelectCallBack(&text_label, selection_callback);
  
  
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
