#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Scene.h"
#include "Texture.h"

class Renderer
{
    public:
        virtual ~Renderer() {}
        virtual void Render(Scene &scene) = 0;

    public:
        glm::ivec2 viewport = glm::ivec2(800, 800);
	    GLuint renderTexture;
};

#endif //__RENDERER_H__