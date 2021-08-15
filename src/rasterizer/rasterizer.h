#ifndef __RASTERIZER_H__
#define __RASTERIZER_H__

#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"
#include "../opengl/Camera.h"

class Rasterizer
{
    public:
        void Render(uint8_t* pixels);
        
        void flip_vertically(uint8_t* pixels);
        
        glm::ivec2 size = glm::ivec2(800, 800);
        int width;
        int height;

        Camera *camera = NULL;
};

#endif //__RASTERIZER_H__