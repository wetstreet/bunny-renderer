#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "../opengl/Scene.h"
#include "../opengl/Texture.h"
#include "../opengl/Skybox.h"

class Renderer
{
    public:
        virtual void Render(Scene &scene) = 0;

    protected:
        Skybox skybox;

    public:
        glm::ivec2 viewport = glm::ivec2(800, 800);
	    GLuint renderTexture;
};

#endif //__RENDERER_H__