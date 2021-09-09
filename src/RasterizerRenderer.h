#ifndef __RASTERIZER_RENDERER_H__
#define __RASTERIZER_RENDERER_H__

#include <vector>
#include <iostream>

#include "common/Renderer.h"
#include "rasterizer/our_gl.h"
#include "opengl/Scene.h"

class RasterizerRenderer : public Renderer
{
    public:
        virtual void Render(Scene &scene);
        
    private:
        uint8_t* Rasterize(Scene &scene);
        void Clear(uint8_t* pixels, glm::vec3 &clearColor);
        void flip_vertically(uint8_t* pixels);
};

#endif //__RASTERIZER_RENDERER_H__