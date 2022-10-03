#pragma once

#include <vector>
#include <string>

#include "vectors.h"

std::string remove_chars(const std::string& input, char c);

std::vector<std::string> split(const std::string& input, const std::string& delimiter);

std::vector<std::string> split_vectors(const std::string& input);

vec3f make_vector(const std::string& input);