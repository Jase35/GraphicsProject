#pragma once

namespace engine{
    struct Camera{
        // TODO use the global transform already in the gameObject.
        float near = 0.01f;
        float far = 1000.0f;
        float fov = 0.625f; // In radians.
    };
}