#pragma once

#include <string>

unsigned int loadShaders(const std::string& VertexShaderCode, const std::string& FragmentShaderCode, const std::string& GeometryShaderCode = "");
