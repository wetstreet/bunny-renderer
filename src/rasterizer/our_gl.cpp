#include <cmath>
#include <limits>
#include <cstdlib>
#include "our_gl.h"

Matrix ModelView;
Matrix Projection;
Matrix Viewport;

IShader::~IShader() {}


void lookat(glm::vec3 eye, glm::vec3 center, glm::vec3 up)
{
    glm::vec3 z = glm::normalize(eye - center);
    glm::vec3 x = glm::normalize(cross(up, z));
    glm::vec3 y = glm::normalize(cross(z, x));
    ModelView = Matrix::identity();
    for (int i=0; i<3; i++)
    {
        ModelView[0][i] = x[i];
        ModelView[1][i] = y[i];
        ModelView[2][i] = z[i];
        ModelView[i][3] = -center[i];
    }
}

void projection(float coeff)
{
    Projection = Matrix::identity();
    Projection[3][2] = coeff;
}

const int depth  = 255;
void viewport(int x, int y, int w, int h)
{
    Viewport = Matrix::identity();
    Viewport[0][3] = x + w / 2.0f;
    Viewport[1][3] = y + h / 2.0f;
    Viewport[2][3] = depth / 2.0f;

    Viewport[0][0] = w / 2.0f;
    Viewport[1][1] = h / 2.0f;
    Viewport[2][2] = depth / 2.0f;
}

Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P)
{
    // P = (1 - u - v)A + uB + vC
    // P = A + u(AB) + v(AC)
    // PA = A - P = -(u(AB) + v(AC))
    // u(AB) + v(AC) + PA = 0
    // (u,v,1) ⊥ (AB.x, AC.x, PA.x)
    // (u,v,1) ⊥ (AB.y, AC.y, PA.y)
    // (u,v,1) = (AB.x, AC.x, PA.x) × (AB.y, AC.y, PA.y)
    Vec3f a = Vec3f(B.x - A.x, C.x - A.x, A.x - P.x);
    Vec3f b = Vec3f(B.y - A.y, C.y - A.y, A.y - P.y);
    Vec3f u = cross(a, b);

    // if u[2] is 0 means triangle is degenerate
    if (std::abs(u[2]) < 1)
        return Vec3f(-1, 1, 1);

    return Vec3f(1.0f - (u.x + u.y)/u.z, u.x/u.z, u.y/u.z);
}

void triangle(Vec4f *pts, IShader &shader, uint8_t* pixels, int *zbuffer, int width, int height)
{
    Vec2f bboxmin = Vec2f(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax = Vec2f(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin[j] = std::min(pts[i][j] / pts[i][3], bboxmin[j]);
            bboxmax[j] = std::max(pts[i][j] / pts[i][3], bboxmax[j]);
        }
    }
    Vec2i P;
    TGAColor color;
    for (P.x = bboxmin.x; P.x < bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y < bboxmax.y; P.y++)
        {
            if (P.x < 0 || P.y < 0 || P.x >= width || P.y >= height) continue;
            
            Vec3f c = barycentric(proj<2>(pts[0]/pts[0][3]), proj<2>(pts[1]/pts[1][3]), proj<2>(pts[2]/pts[2][3]), P);
            float z = pts[0][2] * c.x + pts[1][2] * c.y+ pts[2][2] * c.z;
            float w = pts[0][3] * c.x + pts[1][3] * c.y+ pts[2][3] * c.z;
            float frag_depth = std::max(0, std::min(255, int(z/w + 0.5)));
            int index = P.x + P.y * width;

            if (c.x < 0 || c.y < 0 || c.z < 0 || zbuffer[index] > frag_depth) continue;

            bool discard = shader.fragment(c, color);
            if (!discard)
            {
                pixels[index * 3] = color.bgra[2];
                pixels[index * 3 + 1] = color.bgra[1];
                pixels[index * 3 + 2] = color.bgra[0];
                zbuffer[index] = frag_depth;
            }
        }
    }
}
