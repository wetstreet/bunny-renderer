#ifndef __RAYTRACER_RENDERER_H__
#define __RAYTRACER_RENDERER_H__

#include "common/Renderer.h"

#include "raytracer/rtweekend.h"

#include "raytracer/hittable_list.h"
#include "raytracer/sphere.h"
#include "raytracer/raytracer_camera.h"
#include "raytracer/material.h"
#include "raytracer/moving_sphere.h"

class RayTracerRenderer : public Renderer {
public:
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

    hittable_list random_scene() {
        hittable_list world;

        auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
        world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

        for (int a = -11; a < 11; a++) {
            for (int b = -11; b < 11; b++) {
                auto choose_mat = random_double();
                point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

                if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                    shared_ptr<material> sphere_material;

                    if (choose_mat < 0.8) {
                        auto albedo = color::random() * color::random();
                        sphere_material = make_shared<lambertian>(albedo);
                        auto center2 = center + vec3(0, random_double(0, 0.5), 0);
                        world.add(make_shared<moving_sphere>(center, center2, 0.0, 1.0, 0.2, sphere_material));
                    }
                    else if (choose_mat < 0.95) {
                        auto albedo = color::random(0.5, 1);
                        auto fuzz = random_double(0, 0.5);
                        sphere_material = make_shared<metal>(albedo, fuzz);
                        world.add(make_shared<sphere>(center, 0.2, sphere_material));
                    }
                    else {
                        sphere_material = make_shared<dielectric>(1.5);
                        world.add(make_shared<sphere>(center, 0.2, sphere_material));
                    }
                }
            }
        }

        auto material1 = make_shared<dielectric>(1.5);
        world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

        auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
        world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

        auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
        world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

        return world;
    }

    uint8_t* RayTrace(Scene& scene)
    {
        //Iamge
        const int samples_per_pixel = 1;
        const int max_depth = 4;

        // World
        auto world = random_scene();

        // Camera
        point3 lookfrom(13, 2, 3);
        point3 lookat(0, 0, 0);
        vec3 vup(0, 1, 0);
        auto dist_to_focus = 10.0;
        auto aperture = 0.1;
        const float aspect_ratio = viewport.x / viewport.y;

        raytracer_camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

        // Render
        uint8_t* pixels = new uint8_t[viewport.x * viewport.y * 4];

        for (int j = viewport.y - 1; j >= 0; j--)
        {
            std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
            for (int i = 0; i < viewport.x; i++)
            {
                color pixel_color(0, 0, 0);
                for (int s = 0; s < samples_per_pixel; s++) {
                    auto u = (i + random_double()) / (viewport.x - 1);
                    auto v = (j + random_double()) / (viewport.y - 1);
                    ray r = cam.get_ray(u, v);
                    pixel_color += ray_color(r, world, max_depth);
                }

                int index = i + j * viewport.x;

                auto r = pixel_color.x();
                auto g = pixel_color.y();
                auto b = pixel_color.z();

                auto scale = 1.0 / samples_per_pixel;
                r = sqrt(scale * r);
                g = sqrt(scale * g);
                b = sqrt(scale * b);

                pixels[index * 4] = static_cast<int>(256 * clamp(r, 0.0, 0.999));
                pixels[index * 4 + 1] = static_cast<int>(256 * clamp(g, 0.0, 0.999));
                pixels[index * 4 + 2] = static_cast<int>(256 * clamp(b, 0.0, 0.999));
                pixels[index * 4 + 3] = 255;
            }
        }

        std::cerr << "\nDone.\n";

        flip_vertically(pixels, viewport.x, viewport.y);
        return pixels;
    }

    virtual void RayTracerRenderer::Render(Scene& scene)
    {
        uint8_t* pixels = RayTrace(scene);

        // Create a OpenGL texture identifier
        glGenTextures(1, &renderTexture);
        glBindTexture(GL_TEXTURE_2D, renderTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport.x, viewport.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        delete[] pixels;
    }
};

#endif // __RAYTRACER_RENDERER_H__