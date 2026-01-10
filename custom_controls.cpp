#include "custom_controls.h"

#include <iostream>
#include <algorithm>

#include <imgui.h>

void geom_view_control::newline() {
  //std::cout << "!!!!!!!!!!!!!!!!!!this=" << this << '\n';
  line++;
  //std::cout << "!!!!!!!!!!!!!!!!!!this=" << this << " line=" << line << ':' << &line << '\n';
}

void geom_view_control::add(const std::shared_ptr<geom_view_control>& child) {
  children.push_back(child);
}

void geom_view_control::clear() {
  children.clear();
}

bool geom_view_control::remove(const std::shared_ptr<geom_view_control>& child) {
  auto it = children.begin();
  auto to_remove = children.end();
  for(; it != children.end(); it++) {
    if(*it == child)
      to_remove = it;
  }
  if(to_remove != children.end()) {
    children.erase(to_remove);
    return true;
  } else
  return false;
}


//---------------------panel

std::shared_ptr<geom_view_control_panel> geom_view_control_panel::makeCustomPanel(const std::string& name) {
  std::shared_ptr<geom_view_control_panel> res(new geom_view_control_panel());
  res->name = name;
  return res;
}

void geom_view_control_panel::display(postponed_callbacks_type& ppc) {
  ImGui::Begin(name.c_str());
  if(children.size()) {
    int line = children[0]->line;
    for(auto& child : children) {
      if(!child->visible)
        continue;
      if(child->line != line)
        line = child->line;
      else
        ImGui::SameLine();
      child->display(ppc);
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

void geom_view_control_button::display(postponed_callbacks_type& ppc) {
  ImGui::PushID(reinterpret_cast<size_t>(this));
  bool res = ImGui::Button(name.c_str());
  ImGui::PopID(); 
  if(res && callback)
    ppc.emplace_back(postponed_callback_type{callback, callback_data});
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

void geom_view_control_slidevalue::display(postponed_callbacks_type& ppc) {
  ImGui::PushID(reinterpret_cast<size_t>(this));
  bool res = ImGui::SliderFloat(name.c_str(), &value, vmin, vmax);
  ImGui::PopID();
  if(res && callback)
    ppc.emplace_back(postponed_callback_type{callback, callback_data});
}

//---------------------------int input

std::shared_ptr<geom_view_control_inputInt> geom_view_control_inputInt::makeCustomInputInt(const std::string& name) {
  std::shared_ptr<geom_view_control_inputInt> res(new geom_view_control_inputInt());
  res->name = name;
  return res;
}

void geom_view_control_inputInt::display(postponed_callbacks_type& ppc) {
  ImGui::PushID(reinterpret_cast<size_t>(this));
  bool res = ImGui::InputInt(name.c_str(), &value);
  ImGui::PopID();
  if(res && callback)
    ppc.emplace_back(postponed_callback_type{callback, callback_data});
}

//------------------------------textLabel

std::shared_ptr<geom_view_control_textLabel> geom_view_control_textLabel::makeCustomTextLabel(const std::string& name) {
  std::shared_ptr<geom_view_control_textLabel> res(new geom_view_control_textLabel());
  res->name = name;
  return res;
}

void geom_view_control_textLabel::display(postponed_callbacks_type& ppc) {
  ImGui::PushID(reinterpret_cast<size_t>(this));
  ImGui::Text(name.c_str());
  ImGui::PopID();
  if(callback)
    ppc.emplace_back(postponed_callback_type{callback, callback_data});
}

//---------------------------list
std::shared_ptr<geom_view_control_list> geom_view_control_list::makeCustomList() {
  std::shared_ptr<geom_view_control_list> res(new geom_view_control_list());
  res->name = "LIST";
  return res;
}

void geom_view_control_list::display(postponed_callbacks_type& ppc) {
  ImGui::PushID(reinterpret_cast<size_t>(this));
  static ImGuiTableFlags flags =
      ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
      | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
      | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody
      | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
      | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings;
  int n = std::max((int)header.size(), 1);
  bool selected_status_changed{};
  bool re_sorted{};
  if(ImGui::BeginTable("table", n, flags, ImVec2(.0f, .0f), .0f)) {
    if(header.size() == n) {
      for(int i = 0; i < n; i++) {
        ImGui::TableSetupColumn(header[i].c_str(), ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, reinterpret_cast<size_t>(&header[i]));
      }
    }
    ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs();
    if(sorts_specs->SpecsDirty) {
      for(int i=0;i<sorts_specs->SpecsCount;i++) {
        const ImGuiTableColumnSortSpecs& spec = sorts_specs->Specs[i];
        int col = spec.ColumnIndex;
        bool dir = spec.SortDirection == 2 ? true : false;
        std::stable_sort(items.begin(), items.end(), [&col, &dir](const geom_view_control_list::item& a,
                                                                  const geom_view_control_list::item& b) {
                                                                    return dir ? a.props[col] < b.props[col] : a.props[col] > b.props[col];
                                                                  });
        re_sorted = true;
      }
    }
    sorts_specs->SpecsDirty = false;
    ImGui::TableHeadersRow();
    for(int i_id = 0; i_id < items.size(); i_id++) {
      auto& i = items[i_id];
      bool initial_selected_status = i.selected;
      if(i.props.size() != n)
        continue;
      ImGui::PushID(reinterpret_cast<size_t>(i.ptr));
      ImGui::TableNextRow(ImGuiTableRowFlags_None, .0f);
      for(int r=0; r<n; r++) {
        ImGui::TableSetColumnIndex(r);
        ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_AllowItemOverlap;
        if(spanWholeRow)
          selectable_flags |= ImGuiSelectableFlags_SpanAllColumns;
        bool selected_prev_state = i.selected;
        bool hit = ImGui::Selectable(i.props[r].c_str(), i.selected, selectable_flags, ImVec2(0.f, .0f));
        if(!hit)
          continue;
        last_row_hit = r;
        last_item_hit = i_id;
        if(do_not_togle_selection)
          selected_status_changed = true;

        i.selected ^= hit;

        if (!do_not_togle_selection && !ImGui::GetIO().KeyCtrl) {
          for(int j_id = 0; j_id < items.size(); j_id++) {
            if(i_id!=j_id) {
              selected_status_changed = items[j_id].selected ? true : selected_status_changed;
              items[j_id].selected = false;
            }
          }
          i.selected = true;
        }

        if(i.selected != selected_prev_state)
          selected_status_changed = true;
      }
      if(do_not_togle_selection)
        i.selected = initial_selected_status;
      ImGui::PopID();
    }
    ImGui::EndTable();
  }
  ImGui::PopID();
  if((selected_status_changed || re_sorted) && callback)
    ppc.emplace_back(postponed_callback_type{callback, callback_data});
}

//-------------------------------RADIO

std::shared_ptr<geom_view_control_radio> geom_view_control_radio::makeCustomRadio(const std::vector<std::string>& _names) {
  geom_view_control_radio* res = new geom_view_control_radio();
  res->name = "RADIO";
  for(auto& n : _names)
    res->names.emplace_back(n);
  return std::shared_ptr<geom_view_control_radio>(res);
}

void geom_view_control_radio::display(postponed_callbacks_type& ppc) {
  bool checked = false;
  for(int i=0;i<names.size();i++) {
    ImGui::PushID(&names[i]);
    checked |= ImGui::RadioButton(names[i].c_str(), &choice, i);
    if(i<names.size()-1)
      ImGui::SameLine();
    ImGui::PopID();
  }
  if(checked && callback)
    ppc.emplace_back(postponed_callback_type{callback, callback_data});
}

//-------------------------------CHECKBOX

std::shared_ptr<geom_view_control_checkbox> geom_view_control_checkbox::makeCustomCheckBox(const std::string& _name) {
  geom_view_control_checkbox* res = new geom_view_control_checkbox();
  res->name = _name;
  return std::shared_ptr<geom_view_control_checkbox>(res);
}

void geom_view_control_checkbox::display(postponed_callbacks_type& ppc) {
  ImGui::PushID(&name);
  if(ImGui::Checkbox(name.c_str(), &checked) && callback)
    ppc.emplace_back(postponed_callback_type{callback, callback_data});
  ImGui::PopID();
}

//----------------------TEXTINPUT------------------------
std::shared_ptr<geom_view_control_textinput> geom_view_control_textinput::makeCustomTextInput(const std::string& _name, int size) {
  geom_view_control_textinput* res = new geom_view_control_textinput();
  res->text.resize(size, 0);
  res->name = _name;
  return std::shared_ptr<geom_view_control_textinput>(res);
}

void geom_view_control_textinput::display(postponed_callbacks_type& ppc) {
  ImGui::PushID(&name);
  if(ImGui::InputText(name.c_str(), text.data(), text.size()) && callback)
    ppc.emplace_back(postponed_callback_type{callback, callback_data});
  ImGui::PopID();
}

std::string geom_view_control_textinput::getText() const {
  for(auto it=text.begin(); it!=text.end(); it++) {
    if(*it == 0)
      return std::string(text.begin(), it);
  }
  return std::string();
}

void geom_view_control_textinput::setText(const std::string& in) {
  if(text.size() < in.size())
    text.resize(in.size()+1);
  auto itv = text.begin();
  for(auto it = in.begin(); it!= in.end(); it++, itv++)
    *itv = *it;
  *itv = 0; //trailing zero
}
