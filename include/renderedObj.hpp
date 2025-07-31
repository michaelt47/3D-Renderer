#ifndef RENDEREDOBJ_HPP
#define RENDEREDOBJ_HPP

#include "triangle.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using std::vector, std::ifstream, std::string;

struct HitIndexAndDistance {
    float t;
    int hitIndex;
};

class RenderedObj {
public:
    vector<Triangle> triangles;
    vector<Vec3> normals;

    Vec3 color;
    float reflectiveness;
    int specular;

    RenderedObj();

    void loadObjFile(const string &filename);

    bool intersectRayTriangle(const Vec3 &origin, const Vec3 &dir, const Triangle &triangle, const Vec3 &normal, float tMax, float &tOut);

    HitIndexAndDistance checkRayIntersect(Vec3 origin, Vec3 dir, float tMax);
};

#endif