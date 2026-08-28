#pragma once

#include <charconv>
#include <cstdio>
#include <string>
#include <initializer_list>

#include <glm/glm.hpp>

using namespace glm;
//using namespace engine;

namespace debug{
    std::string pretty(float value);
    std::string pretty(std::initializer_list<float> values);
    void printMat4(const std::string name, const mat4& inst);
    void printVec3(const std::string name, const vec3& inst);
}