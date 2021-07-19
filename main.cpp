#include <vector>
#include <cmath>
#include <limits>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white    = TGAColor(255, 255, 255, 255);
const TGAColor red      = TGAColor(255, 0, 0, 255);
const TGAColor green    = TGAColor(0, 255, 0, 255);
Model *model = NULL;
const int width = 800;
const int height = 800;

void line(Vec2i v0, Vec2i v1, TGAImage &image, TGAColor color)
{
    bool steep = false;
    if (std::abs(v1.y - v0.y) > std::abs(v1.x - v0.x)) // from bottom to top
    {
        std::swap(v0.x, v0.y);
        std::swap(v1.x, v1.y);
        steep = true;
    }
    if (v1.x < v0.x) // from left to right
    {
        std::swap(v1.x, v0.x);
        std::swap(v1.y, v0.y);
    }
    for (int x = v0.x; x <= v1.x; x++)
    {
        float t = (x - v0.x) / (v1.x - v0.x);
        float y = v0.y + (v1.y - v0.y) * t;
        if (steep)
            image.set(y, x, color);
        else
            image.set(x, y, color);
    }
}

Vec3f barycentric(Vec3f *pts, Vec3f P)
{
    // P = (1 - u - v)A + uB + vC
    // P = A + u(AB) + v(AC)
    // PA = A - P = -(u(AB) + v(AC))
    // u(AB) + v(AC) + PA = 0
    // (u,v,1) ⊥ (AB.x, AC.x, PA.x)
    // (u,v,1) ⊥ (AB.y, AC.y, PA.y)
    // (u,v,1) = (AB.x, AC.x, PA.x) × (AB.y, AC.y, PA.y)
    Vec3f a = Vec3f(pts[1][0] - pts[0][0], pts[2][0] - pts[0][0], pts[0][0] - P[0]);
    Vec3f b = Vec3f(pts[1][1] - pts[0][1], pts[2][1] - pts[0][1], pts[0][1] - P[1]);
    Vec3f u = cross(a, b);

    // if u[2] is 0 means triangle is degenerate
    if (std::abs(u[2]) < 1)
        return Vec3f(-1, 1, 1);

    return Vec3f(1.0f - (u.x + u.y)/u.z, u.x/u.z, u.y/u.z);
}

void triangle(Vec3f *pts, Vec3f *uvs, float *zbuffer, TGAImage &diffuse, TGAImage &image, float nl)
{
    Vec2f bboxmin = Vec2f(image.get_width() - 1, image.get_height() - 1);
    Vec2f bboxmax = Vec2f(0,0);
    Vec2f clamp = Vec2f(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin[j] = std::max(std::min(pts[i][j], bboxmin[j]), 0.0f);
            bboxmax[j] = std::min(std::max(pts[i][j], bboxmax[j]), clamp[j]);
        }
    }
    Vec3f P;
    Vec3f uv;
    for (P.x = bboxmin.x; P.x < bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y < bboxmax.y; P.y++)
        {
            Vec3f bc_screen = barycentric(pts, P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;

            P.z = 0;
            for (int i = 0; i < 3; i++)
            {
                uv[i] = 0;
            }
            for (int i = 0; i < 3; i++)
            {
                P.z += bc_screen[i] * pts[i][2];
                for (int j = 0; j < 3; j++)
                {
                    uv[j] += bc_screen[i] * uvs[i][j];
                }
            }
            int zindex = P.x + P.y * width;
            if (zbuffer[zindex] < P.z)
            {
                zbuffer[zindex] = P.z;

                uv.y = 1 - uv.y;
                TGAColor diffcolor = diffuse.get(uv.x * diffuse.get_width(), uv.y * diffuse.get_height());

                image.set(P.x, P.y, diffcolor * nl);
            }
        }
    }
}

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        model = new Model(argv[1]);
    }
    else
    {
        model = new Model("obj/african_head/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);

    TGAImage diffuse = TGAImage();
    diffuse.read_tga_file("obj/african_head/african_head_diffuse.tga");

    Vec3f lightDir(0, 0, -1);

    float *zbuffer = new float[width*height];
    for (int i = 0; i < width * height; i++)
    {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }
    
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vec3f screen_coords[3];
        Vec3f world_coords[3];
        Vec3f uv_coords[3];
        for (int j = 0; j < 3; j++)
        {
            world_coords[j] = model->vert(face[j * 2]);
            uv_coords[j] = model->uv(face[j * 2 + 1]);
            screen_coords[j] = Vec3f(int((world_coords[j].x + 1) * width / 2), int((world_coords[j].y + 1) * height / 2), world_coords[j].z);
        }
        Vec3f n = cross((world_coords[2] - world_coords[0]), (world_coords[1] - world_coords[0]));
        n.normalize();
        float nl = n * lightDir;
        if (nl > 0)
        {
            triangle(screen_coords, uv_coords, zbuffer, diffuse, image, nl);
        }
    }

    std::cout << "finish";

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}