#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include "hittable.h"
#include "vec3.h"
#include "glm/glm.hpp"

// Triangle Declarations
struct TriangleMesh {
    // TriangleMesh Public Methods
    TriangleMesh(const glm::mat4& ObjectToWorld, int nTriangles,
        const GLuint* vertexIndices, int nVertices, const glm::vec3* P,
        const glm::vec3* S, const glm::vec3* N, const glm::vec2* UV, std::shared_ptr<material> material)
        : nTriangles(nTriangles),
        nVertices(nVertices),
        vertexIndices(vertexIndices, vertexIndices + 3 * nTriangles),
        material(material)
    {
        // Transform mesh vertices to world space
        p.reset(new glm::vec3[nVertices]);

        for (int i = 0; i < nVertices; ++i) p[i] = ObjectToWorld * glm::vec4(P[i], 1);

        // Copy _UV_, _N_, and _S_ vertex data, if present
        if (UV) {
            uv.reset(new glm::vec2[nVertices]);
            memcpy(uv.get(), UV, nVertices * sizeof(glm::vec2));
        }
        if (N) {
            n.reset(new glm::vec3[nVertices]);
            for (int i = 0; i < nVertices; ++i) n[i] = ObjectToWorld * glm::vec4(N[i], 0);
        }
        if (S) {
            s.reset(new glm::vec3[nVertices]);
            for (int i = 0; i < nVertices; ++i) s[i] = ObjectToWorld * glm::vec4(S[i], 0);
        }
    }

    // TriangleMesh Data
    const int nTriangles, nVertices;
    std::vector<GLuint> vertexIndices;
    std::unique_ptr<glm::vec3[]> p;
    std::unique_ptr<glm::vec3[]> n;
    std::unique_ptr<glm::vec3[]> s;
    std::unique_ptr<glm::vec2[]> uv;
    std::shared_ptr<material> material;
};

class triangle : public hittable {
public:
    triangle() {}
    triangle(const std::shared_ptr<TriangleMesh>& mesh, int triNumber)
    {
        const GLuint* v = &mesh->vertexIndices[triNumber * 3];
        for (int i = 0; i < 3; i++)
        {
            auto& vert = mesh->p[v[i]];
            p[i] = point3(vert.x, vert.y, vert.z);
            auto& normal = mesh->n[v[i]];
            n[i] = vec3(normal.x, normal.y, normal.z);
        }
        mat_ptr = mesh->material;
    }

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
    virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

public:
    point3 p[3];
    vec3 n[3];
    shared_ptr<material> mat_ptr;
};

std::vector<std::shared_ptr<triangle>> CreateTriangleMesh(
    const glm::mat4* ObjectToWorld, int nTriangles, const GLuint* vertexIndices,
    int nVertices, const glm::vec3* p, const glm::vec3* s, const glm::vec3* n,
    const glm::vec2* uv, std::shared_ptr<material> material) {
    std::shared_ptr<TriangleMesh> mesh = std::make_shared<TriangleMesh>(
        *ObjectToWorld, nTriangles, vertexIndices, nVertices, p, s, n, uv, material);
    std::vector<std::shared_ptr<triangle>> tris;
    tris.reserve(nTriangles);
    for (int i = 0; i < nTriangles; ++i)
        tris.push_back(std::make_shared<triangle>(mesh, i));
    return tris;
}

bool triangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {

    const point3& p0 = p[0];
    const point3& p1 = p[1];
    const point3& p2 = p[2];

    // Translate vertices based on ray origin
    point3 p0t = p0 - r.orig;
    point3 p1t = p1 - r.orig;
    point3 p2t = p2 - r.orig;

    // Permute components of triangle vertices and ray direction
    int kz = max_dimension(abs(r.dir));
    int kx = kz + 1;
    if (kx == 3) kx = 0;
    int ky = kx + 1;
    if (ky == 3) ky = 0;
    vec3 d = permute(r.dir, kx, ky, kz);
    p0t = permute(p0t, kx, ky, kz);
    p1t = permute(p1t, kx, ky, kz);
    p2t = permute(p2t, kx, ky, kz);

    // Apply shear transformation to translated vertex positions
    float Sx = -d[0] / d[2];
    float Sy = -d[1] / d[2];
    float Sz = 1.f / d[2];
    p0t[0] += Sx * p0t[2];
    p0t[1] += Sy * p0t[2];
    p1t[0] += Sx * p1t[2];
    p1t[1] += Sy * p1t[2];
    p2t[0] += Sx * p2t[2];
    p2t[1] += Sy * p2t[2];

    // Compute edge function coefficients _e0_, _e1_, and _e2_
    float e0 = p1t[0] * p2t[1] - p1t[1] * p2t[0];
    float e1 = p2t[0] * p0t[1] - p2t[1] * p0t[0];
    float e2 = p0t[0] * p1t[1] - p0t[1] * p1t[0];

    // Perform triangle edge and determinant tests
    if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
        return false;
    float det = e0 + e1 + e2;
    if (det == 0) return false;

    // Compute scaled hit distance to triangle and test against ray $t$ range
    p0t[2] *= Sz;
    p1t[2] *= Sz;
    p2t[2] *= Sz;
    float tScaled = e0 * p0t[2] + e1 * p1t[2] + e2 * p2t[2];
    if (det < 0 && (tScaled >= 0 || tScaled < t_max * det))
        return false;
    else if (det > 0 && (tScaled <= 0 || tScaled > t_max * det))
        return false;

    // Compute barycentric coordinates and $t$ value for triangle intersection
    float invDet = 1 / det;
    float b0 = e0 * invDet;
    float b1 = e1 * invDet;
    float b2 = e2 * invDet;
    float t = tScaled * invDet;

    // Interpolate $(u,v)$ parametric coordinates and hit point
    point3 pHit = b0 * p0 + b1 * p1 + b2 * p2;
    vec3 nHit = b0 * n[0] + b1 * n[0] + b2 * n[0];
    //Point2f uvHit = b0 * uv[0] + b1 * uv[1] + b2 * uv[2];


    rec.t = t;
    rec.p = r.at(rec.t);
    rec.set_face_normal(r, nHit);
    rec.mat_ptr = mat_ptr;

    return true;
}

bool triangle::bounding_box(double time0, double time1, aabb& output_box) const {
    point3 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    point3 max(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (p[i][j] < min[j])
                min[j] = p[i][j];
            if (p[i][j] > max[i])
                max[j] = p[i][j];
        }
    }
    output_box = aabb(min, max);
    return true;
}

#endif //__TRIANGLE_H__