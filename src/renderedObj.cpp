#include "../include/renderedObj.hpp"
#include <iostream>

static float INF = 16777215;

RenderedObj::RenderedObj() {}

void RenderedObj::loadObjFile(const string &filename)
{
    vector<Vec3> vertices;
    ifstream file(filename);
    string line;

    while (std::getline(file, line))
    {
        std::istringstream iss(line);

        if (line.substr(0, 2) == "v ")
        {
            char v_char;
            Vec3 vertex;
            iss >> v_char >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        }
        else if (line.substr(0, 2) == "f ")
        {
            std::vector<int> indices;
            std::string token;

            std::string dummy;
            iss >> dummy; // skip the "f"

            while (iss >> token)
            {
                std::istringstream tss(token);
                std::string index_str;
                std::getline(tss, index_str, '/'); // handle v/vt/vn
                int idx = std::stoi(index_str);
                indices.push_back(idx - 1); // .obj is 1-based index
            }

            for (size_t i = 1; i + 1 < indices.size(); ++i)
            {
                Triangle tri;
                tri.v0 = vertices[indices[0]];
                tri.v1 = vertices[indices[i]];
                tri.v2 = vertices[indices[i + 1]];
                triangles.push_back(tri);

                Vec3 A = tri.v1 - tri.v0;
                Vec3 B = tri.v2 - tri.v0;
                Vec3 N = A.cross(B).normalized();
                normals.push_back(N);
            }
        }
    }

    for (Triangle &triangle : triangles) {
        triangle.center = (triangle.v0 + triangle.v1 + triangle.v2) * (1/3);
    }
}

bool RenderedObj::intersectRayTriangle(const Vec3 &origin, const Vec3 &dir, const Triangle &triangle, const Vec3 &normal, float tMax, float &tOut) {
    float denom = normal.dot(dir);
    if (fabs(denom) < 0.05f) return false;

    float t = (triangle.center - origin).dot(normal) / denom;
    if (t < 0.001f || t > tMax) return false;

    Vec3 P = origin + dir * t;

    // Get triangle vertices
    const Vec3& v0 = triangle.v0;
    const Vec3& v1 = triangle.v1;
    const Vec3& v2 = triangle.v2;

    // Compute vectors
    Vec3 v0v1 = v1 - v0;
    Vec3 v0v2 = v2 - v0;
    Vec3 v0p  = P - v0;

    // Compute dot products
    float d00 = v0v1.dot(v0v1);
    float d01 = v0v1.dot(v0v2);
    float d11 = v0v2.dot(v0v2);
    float d20 = v0p.dot(v0v1);
    float d21 = v0p.dot(v0v2);

    // Compute barycentric coordinates
    float denomTri = d00 * d11 - d01 * d01;
    float v = (d11 * d20 - d01 * d21) / denomTri;
    float w = (d00 * d21 - d01 * d20) / denomTri;
    float u = 1.0f - v - w;

    // Check if point is inside the triangle
    if (u >= 0 && v >= 0 && w >= 0) {
        tOut = t;
        return true;
    }

    return false;
}

HitIndexAndDistance RenderedObj::checkRayIntersect(Vec3 origin, Vec3 dir, float tMax) {
    float closestIntersectionDistance = INF;
    int hitIndex = -1;

    for (int i = 0; i < triangles.size(); i++) {
        float t;
        if (intersectRayTriangle(origin, dir, triangles[i], normals[i], tMax, t)) {
            if (t < closestIntersectionDistance) {
                closestIntersectionDistance = t;
                hitIndex = i;
            }
        }
    }

    return {closestIntersectionDistance, hitIndex};
}