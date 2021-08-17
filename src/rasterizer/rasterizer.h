#ifndef __RASTERIZER_H__
#define __RASTERIZER_H__

#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "our_gl.h"
#include "../opengl/Scene.h"

class Rasterizer
{
    public:
        void Render(uint8_t* pixels);
        
        void Clear(uint8_t* pixels, glm::vec3 &clearColor);
        void flip_vertically(uint8_t* pixels);
        
        glm::ivec2 size = glm::ivec2(800, 800);
        int width;
        int height;

        Scene *scene = NULL;
};

#endif //__RASTERIZER_H__