#ifndef __RASTERIZER_H__
#define __RASTERIZER_H__

#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

class Rasterizer
{
    public:
        void Render(uint8_t* pixels);
        
        const int width = 800;
        const int height = 800;
};

#endif //__RASTERIZER_H__