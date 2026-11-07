#pragma once

#include <glm/glm.hpp>

using namespace glm;

namespace engine{
    class Ray{
        public:
        Ray(){}
        Ray(const vec3& orig, const vec3& dir) : origin(orig), direction(dir) {}
        const vec3& origin() const { return origin; }
        const vec3& direction() const { return direction; }
        vec3 at(float t){
            return origin + (direction * t);
        }

        private:
        vec3 origin;
        vec3 direction;
    };
}