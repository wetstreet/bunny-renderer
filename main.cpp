#include <vector>
#include <cmath>
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

    int dx = v1.x - v0.x;
    int dy = v1.y - v0.y;
    int derror = std::abs(dy);
    int error = 0;
    int y = v0.y;
    for (int x = v0.x; x <= v1.x; x++)
    {
        if (steep)
            image.set(y, x, color);
        else
            image.set(x, y, color);
        error += derror;
        if (error > dx / 2)
        {
            y += v1.y > v0.y ? 1 : -1;
            error -= dx;
        }
    }
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color)
{
    if (t0.y > t1.y) std::swap(t0, t1);
    if (t0.y > t2.y) std::swap(t0, t2);
    if (t1.y > t2.y) std::swap(t1, t2);
    
    int total_height = t2.y - t0.y;
    for (int y = t0.y; y < t1.y; y++)
    {
        int segment_height = t1.y - t0.y + 1; // avoid division by zero
        float alpha = (y - t0.y) / (float)total_height;
        float beta = (y - t0.y) / (float)segment_height;
        Vec2i A = t0 + (t2 - t0) * alpha;
        Vec2i B = t0 + (t1 - t0) * beta;
        if (A.x > B.x)
        {
            std::swap(A, B);
        }
        for (int x = A.x; x <= B.x; x++)
        {
            image.set(x, y, color);
        }
    }
    for (int y = t1.y; y < t2.y; y++)
    {
        int segment_height = t2.y - t1.y + 1; // avoid division by zero
        float alpha = (y - t0.y) / (float)total_height;
        float beta = (y - t1.y) / (float)segment_height;
        Vec2i A = t0 + (t2 - t0) * alpha;
        Vec2i B = t1 + (t2 - t1) * beta;
        if (A.x > B.x)
        {
            std::swap(A, B);
        }
        for (int x = A.x; x <= B.x; x++)
        {
            image.set(x, y, color);
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
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    Vec3f lightDir(0, 0, -1);

    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];
        for (int j = 0; j < 3; j++)
        {
            world_coords[j] = model->vert(face[j]);
            screen_coords[j] = Vec2i((world_coords[j].x + 1) * width / 2, (world_coords[j].y + 1) * height / 2);
        }
        Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
        n.normalize();
        float intensity = n * lightDir;
        if (intensity > 0)
        {
            triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
        }
    }

    std::cout << "finish";

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}