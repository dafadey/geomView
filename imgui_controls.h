#pragma once

#include <vector>

struct renderer;

struct object;

namespace ImGui {
  bool ObjectsControl(object* obj, renderer* ren);

  bool CameraControl(renderer* ren);
}
