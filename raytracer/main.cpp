#include <iostream>

#include "color.h"
#include "tgaimage.h"

int main()
{
    const int image_width = 256;
    const int image_height = 256;

    TGAImage image(image_width, image_width, TGAImage::RGB);

    // std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = image_height - 1; j >= 0; j--)
    {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; i++)
        {
            color pixel_color(double(i) / (image_width - 1), double(j) / (image_height - 1), 0.25);
            // write_color(std::cout, pixel_color);
            write_color(image, i, j, pixel_color);
        }
    }
    
    std::cerr << "\nDone.\n";
    
    image.flip_vertically();
    image.write_tga_file("output.tga");
}