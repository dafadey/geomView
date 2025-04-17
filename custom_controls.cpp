#include "custom_controls.h"

#include <iostream>

#include <imgui.h>

void geom_view_control::newline() {
  std::cout << "!!!!!!!!!!!!!!!!!!this=" << this << '\n';
  line++;
  std::cout << "!!!!!!!!!!!!!!!!!!this=" << this << " line=" << line << ':' << &line << '\n';
}

void geom_view_control::add(const std::shared_ptr<geom_view_control>& child) {
  children.push_back(child);
}

//---------------------panel

std::shared_ptr<geom_view_control_panel> geom_view_control_panel::makeCustomPanel(const std::string& name) {
  std::shared_ptr<geom_view_control_panel> res(new geom_view_control_panel());
  res->name = name;
  return res;
}

void geom_view_control_panel::display() {
  ImGui::Begin(name.c_str());
  if(children.size()) {
    int line = children[0]->line;
    for(auto& child : children) {
      if(child->line != line)
        line = child->line;
      else
        ImGui::SameLine();
      child->display();
    }
  }
  ImGui::End();
}

//----------------------------button

std::shared_ptr<geom_view_control_button> geom_view_control_button::makeCustomButton(const std::string& name) {
  std::shared_ptr<geom_view_control_button> res(new geom_view_control_button());
  res->name = name;
  return res;
}

void geom_view_control_button::display() {
  ImGui::PushID(reinterpret_cast<size_t>(this));
  bool res = ImGui::Button(name.c_str());
  ImGui::PopID(); 
  if(res && callback)
    callback(callback_data);
}

//---------------------------slidevalue

std::shared_ptr<geom_view_control_slidevalue> geom_view_control_slidevalue::makeCustomSlidevalue(const std::string& name, float vmin, float vmax) {
  std::shared_ptr<geom_view_control_slidevalue> res(new geom_view_control_slidevalue());
  res->name = name;
  res->vmin = vmin;
  res->vmax = vmax;
  res->value = .5 * (vmin + vmax);
  return res;
}

void geom_view_control_slidevalue::display() {
  ImGui::PushID(reinterpret_cast<size_t>(this));
  bool res = ImGui::SliderFloat(name.c_str(), &value, vmin, vmax);
  ImGui::PopID();
  if(res && callback)
    callback(callback_data);
}

//------------------------------textLabel

std::shared_ptr<geom_view_control_textLabel> geom_view_control_textLabel::makeCustomTextLabel(const std::string& name) {
  std::shared_ptr<geom_view_control_textLabel> res(new geom_view_control_textLabel());
  res->name = name;
  return res;
}

void geom_view_control_textLabel::display() {
  ImGui::PushID(reinterpret_cast<size_t>(this));
  ImGui::Text(name.c_str());
  ImGui::PopID();
  if(callback)
    callback(callback_data);
}
