#ifndef VEC3_HPP
#define VEC3_HPP

#include <cmath>

class Vec3 {
public:
    float x, y, z;

    Vec3();
    Vec3(float x, float y, float z);

    Vec3(const Vec3& other);

    Vec3& operator=(const Vec3& other);

    Vec3 operator+(const Vec3& other) const;
    Vec3 operator-(const Vec3& other) const;
    Vec3 operator*(float scalar) const;

    float dot(const Vec3& other) const;
    Vec3 cross(const Vec3& other) const;

    Vec3 normalized() const;
    float mag() const;

};

#endif // VEC3_HPP