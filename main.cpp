#include <vector>
#include <cmath>
#include <limits>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const int width = 800;
const int height = 800;
const int depth  = 255;

Model *model = NULL;
Vec3f lightDir(0, 0, -1);
Vec3f camera(0,0,3);

Matrix v2m(Vec3f v)
{
    Matrix m = Matrix(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1;
    return m;
}

Vec3f m2v(Matrix m)
{
    return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}

Matrix viewport(int x, int y, int w, int h)
{
    Matrix m = Matrix::identity(4);
    m[0][3] = x + w / 2.0f;
    m[1][3] = y + h / 2.0f;
    m[2][3] = depth / 2.0f;

    m[0][0] = w / 2.0f;
    m[1][1] = h / 2.0f;
    m[2][2] = depth / 2.0f;
    return m;
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
    Vec3f u = a ^ b;

    // if u[2] is 0 means triangle is degenerate
    if (std::abs(u[2]) < 1)
        return Vec3f(-1, 1, 1);

    return Vec3f(1.0f - (u.x + u.y)/u.z, u.x/u.z, u.y/u.z);
}

void triangle(Vec3f *pts, Vec3f *uvs, float *intensities, float *zbuffer, TGAImage &diffuse, TGAImage &image)
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
    float intensity;
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
            intensity = 0;
            // lerp
            for (int i = 0; i < 3; i++)
            {
                P.z += bc_screen[i] * pts[i][2];
                for (int j = 0; j < 3; j++)
                {
                    uv[j] += bc_screen[i] * uvs[i][j];
                }
                intensity += bc_screen[i] * intensities[i];
            }
            int zindex = P.x + P.y * width;
            if (zbuffer[zindex] < P.z)
            {
                zbuffer[zindex] = P.z;

                uv.y = 1 - uv.y;
                TGAColor diffcolor = diffuse.get(uv.x * diffuse.get_width(), uv.y * diffuse.get_height());

                image.set(P.x, P.y, diffcolor * intensity);
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

    float *zbuffer = new float[width*height];
    for (int i = 0; i < width * height; i++)
    {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }

    Matrix Projection = Matrix::identity(4);
    Matrix ViewPort = viewport(0, 0, width, height);
    Projection[3][2] = -1.0f / camera.z;
    
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vec3f screen_coords[3];
        Vec3f world_coords[3];
        Vec3f uv_coords[3];
        Vec3f normal_coords[3];
        float intensities[3];
        bool shouldRender = false;
        for (int j = 0; j < 3; j++)
        {
            world_coords[j] = model->vert(face[j * 3]);
            uv_coords[j] = model->uv(face[j * 3 + 1]);
            normal_coords[j] = model->normal(face[j * 3 + 2]);
            intensities[j] = normal_coords[j].normalize() * lightDir;
            intensities[j] = -intensities[j];
            if (intensities[j] > 0)
                shouldRender = true;
            
            screen_coords[j] = m2v(ViewPort * Projection * v2m(world_coords[j]));
            screen_coords[j][0] = (int)(screen_coords[j][0]);
            screen_coords[j][1] = (int)(screen_coords[j][1]);
        }
        if (shouldRender)
            triangle(screen_coords, uv_coords, intensities, zbuffer, diffuse, image);
    }

    std::cout << "finish";

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    delete [] zbuffer;
    return 0;
}