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

#endif //__COLOR_H__