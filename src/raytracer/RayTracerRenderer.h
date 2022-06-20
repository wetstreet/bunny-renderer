#ifndef __RAYTRACER_RENDERER_H__
#define __RAYTRACER_RENDERER_H__

#include "common/Renderer.h"

#include "raytracer/rtweekend.h"

#include "raytracer/hittable_list.h"
#include "raytracer/sphere.h"
#include "raytracer/triangle.h"
#include "raytracer/raytracer_camera.h"
#include "raytracer/material.h"
#include "raytracer/moving_sphere.h"
#include "ThreadPool.h"

class RayTracerRenderer : public Renderer {
public:
    std::unique_ptr<ThreadPool> pool;
    std::unique_ptr<hittable_list> world;
    std::unique_ptr<raytracer_camera> cam;

    // image buffer
    uint8_t* pixels = nullptr;
    std::mutex tile_mutex;

    std::function<void()> m_callback = nullptr;

    int samples_per_pixel = 4;
    int max_depth = 8;

    int tileSize = 16;
    int finishedTileCount = 0;
    int totalTileCount = 0;
    double startTime = 0;

    RayTracerRenderer()
    {
        pool = std::make_unique<ThreadPool>(std::thread::hardware_concurrency());

        viewport.x = 1200;
        viewport.y = 600;

        glGenTextures(1, &renderTexture);
        glBindTexture(GL_TEXTURE_2D, renderTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same
    }

    color ray_color(const ray& r, const hittable& world, int depth) {
        hit_record rec;

        if (depth <= 0)
            return color(0, 0, 0);

        if (world.hit(r, 0.001, infinity, rec)) {
            ray scattered;
            color attenuation;
            if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, world, depth - 1);
            return color(0, 0, 0);
        }
        vec3 unit_direction = unit_vector(r.direction());
        auto t = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
    }

    void RayTrace(Scene& scene)
    {
        if (pixels != nullptr)
        {
            delete[] pixels;
        }
        pixels = new uint8_t[viewport.x * viewport.y * 4];

        // Camera
        point3 lookfrom(13, 2, 3);
        point3 lookat(0, 0, 0);
        vec3 vup(0, 1, 0);
        auto dist_to_focus = 10.0;
        auto aperture = 0.1;
        const float aspect_ratio = (float)viewport.x / viewport.y;

        cam = std::make_unique<raytracer_camera>(scene.camera.Position, scene.camera.Position + scene.camera.Orientation, vup, scene.camera.FOVdeg, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

        int xTiles = (viewport.x + tileSize - 1) / tileSize;
        int yTiles = (viewport.y + tileSize - 1) / tileSize;

        totalTileCount = xTiles * yTiles;
        finishedTileCount = 0;

        auto renderTile = [this](int xTile, int yTile) {
            int xStart = xTile * tileSize;
            int yStart = yTile * tileSize;
            for (int j = yStart; j < yStart + tileSize; j++)
            {
                for (int i = xStart; i < xStart + tileSize; i++)
                {
                    // bounds check
                    if (i >= viewport.x || j >= viewport.y)
                        continue;

                    color pixel_color(0, 0, 0);
                    for (int s = 0; s < samples_per_pixel; s++) {
                        auto u = (i + random_double()) / (viewport.x - 1);
                        auto v = (j + random_double()) / (viewport.y - 1);
                        ray r = cam->get_ray(u, v);
                        pixel_color += ray_color(r, *world, max_depth);
                    }

                    auto scale = 1.0 / samples_per_pixel;
                    auto r = sqrt(scale * pixel_color.x());
                    auto g = sqrt(scale * pixel_color.y());
                    auto b = sqrt(scale * pixel_color.z());

                    int index = i + j * viewport.x;
                    pixels[index * 4] = static_cast<int>(256 * clamp(r, 0.0, 0.999));
                    pixels[index * 4 + 1] = static_cast<int>(256 * clamp(g, 0.0, 0.999));
                    pixels[index * 4 + 2] = static_cast<int>(256 * clamp(b, 0.0, 0.999));
                    pixels[index * 4 + 3] = 255;
                }
            }
            {
                std::lock_guard<std::mutex> lock(tile_mutex);
                finishedTileCount++;
                if (finishedTileCount == totalTileCount)
                {
                    if (m_callback != nullptr)
                    {
                        m_callback();
                    }
                }
            }
        };

        for (int i = 0; i < xTiles; i++)
        {
            for (int j = 0; j < yTiles; j++)
            {
                pool->enqueue(renderTile, i, j);
            }
        }
    }

    void RayTracerRenderer::RenderAsync(Scene& scene, std::function<void()> callback)
    {
        m_callback = callback;

        // create world
        world = std::make_unique<hittable_list>();

        for (std::shared_ptr<Object> obj : scene.objects)
        {
            if (obj->GetType() == Type_Mesh)
            {
                std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(obj);
                auto mat = make_shared<lambertian>(color(mesh->material->color.r, mesh->material->color.g, mesh->material->color.b));
                auto triangles = CreateTriangleMesh(&mesh->objectToWorld, int(mesh->indices.size()) / 3, mesh->indices.data(), int(mesh->vertices.size()), &mesh->verts[0], &mesh->tangents[0], &mesh->normals[0], &mesh->uvs[0], mat);
                for (auto triangle : triangles)
                    world->add(triangle);
            }
        }

        RayTrace(scene);
    }

    // empty implementation, only supprt async render
    virtual void RayTracerRenderer::Render(Scene& scene){}

    ~RayTracerRenderer()
    {
        delete[] pixels;
    }
};

#endif // __RAYTRACER_RENDERER_H__