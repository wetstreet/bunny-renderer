#ifndef __RASTERIZER_RENDERER_H__
#define __RASTERIZER_RENDERER_H__

#include <vector>
#include <iostream>

#include "common/Renderer.h"

class RasterizerRenderer : public Renderer
{
    public:
        virtual void Render(Scene &scene);
        GLuint depthTexture;
        
    private:
        void Rasterize(Scene &scene, uint8_t*pixels, float* zbuffer);
        void Clear(uint8_t* pixels, glm::vec3 &clearColor);
};

#endif //__RASTERIZER_RENDERER_H__