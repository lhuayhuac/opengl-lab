#pragma once
#include "Vec3.h"

class Billboard {
public:
    static void render(const Vec3& center,
                       float width, float height,
                       const Vec3& camRight, const Vec3& camUp,
                       float alpha,
                       bool  useTexture,
                       float r, float g, float b);
};