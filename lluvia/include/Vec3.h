#pragma once
#include <cmath>

struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    // Operadores aritméticos
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(float t)       const { return {x*t,   y*t,   z*t};   }
    Vec3 operator/(float t)       const { return {x/t,   y/t,   z/t};   }
    Vec3 operator-()              const { return {-x, -y, -z};           }

    Vec3& operator+=(const Vec3& o) { x+=o.x; y+=o.y; z+=o.z; return *this; }
    Vec3& operator-=(const Vec3& o) { x-=o.x; y-=o.y; z-=o.z; return *this; }
    Vec3& operator*=(float t)       { x*=t;   y*=t;   z*=t;   return *this; }

    // Producto punto
    float dot(const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }

    // Producto vectorial
    Vec3 cross(const Vec3& o) const {
        return { y*o.z - z*o.y,
                 z*o.x - x*o.z,
                 x*o.y - y*o.x };
    }

    float lengthSq() const { return x*x + y*y + z*z; }
    float length()   const { return sqrtf(lengthSq()); }

    Vec3 normalized() const {
        float l = length();
        return l > 0.0001f ? (*this / l) : Vec3(0,0,0);
    }

    // Interpolación lineal
    static Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
        return a + (b - a) * t;
    }
};