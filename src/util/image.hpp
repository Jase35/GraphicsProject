#pragma once

#include <cstddef>
#include <cstdint>

namespace engine{
    struct Image{
        size_t width = 0, height = 0;
        unsigned char* data = nullptr;

        Image() = default;
        Image(size_t width, size_t height, unsigned char* data) : width(width), height(height), data(data) {}
    };
}