#include "debug.hpp"

namespace debug{
    std::string pretty(float value){
        return std::to_string(value);
    }

    std::string pretty(std::initializer_list<float> values){
        std::string result;
        bool first = true;
        for(const float& value : values){
            if(!first){
                result += ", ";
            }
            first = false;
            result += pretty(value);
        }
        return result;
    }

    void printMat4(const std::string name, const mat4& inst){
        /*printf("%s", (name + ":\n" + 
            pretty({inst.m[0], inst.m[4], inst.m[8], inst.m[12]}) + "\n" +
            pretty({inst.m[1], inst.m[5], inst.m[9], inst.m[13]}) + "\n" +
            pretty({inst.m[2], inst.m[6], inst.m[10], inst.m[14]}) + "\n" +
            pretty({inst.m[3], inst.m[7], inst.m[11], inst.m[15]}) + "\n"
        ).c_str());*/
    }

    void printVec3(const std::string name, const vec3& inst){
        /*printf("%s", (name + ": " + 
            pretty({inst.x, inst.y, inst.z})
        ).c_str());*/
    }
}