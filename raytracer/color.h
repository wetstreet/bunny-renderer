#ifndef __COLOR_H__
#define __COLOR_H__

#include "vec3.h"
#include "tgaimage.h"

#include <iostream>

void write_color(std::ostream &out, color pixel_color) {
    out << static_cast<int>(255.999 * pixel_color.x()) << ' ' << static_cast<int>(255.999 * pixel_color.y()) << ' ' << static_cast<int>(255.999 * pixel_color.z()) << '\n';
}

void write_color(TGAImage &image, int x, int y, color pixel_color) {
    image.set(x, y, TGAColor(static_cast<int>(255.999 * pixel_color.x()),
                             static_cast<int>(255.999 * pixel_color.y()),
                             static_cast<int>(255.999 * pixel_color.z())));
}

void write_color(TGAImage &image, int x, int y, color pixel_color, int samples_per_pixel) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    auto scale = 1.0 / samples_per_pixel;
    r *= scale;
    g *= scale;
    b *= scale;

    image.set(x, y, TGAColor(static_cast<int>(256 * clamp(r, 0.0, 0.999)),
                            static_cast<int>(256 * clamp(g, 0.0, 0.999)),
                            static_cast<int>(256 * clamp(b, 0.0, 0.999))));
}

#endif //__COLOR_H__