#ifndef __OPENGL_RENDERER_H__
#define __OPENGL_RENDERER_H__

#include "common/Renderer.h"

class OpenGLRenderer : public Renderer
{
    public:
        OpenGLRenderer();
        virtual ~OpenGLRenderer();

        virtual void Render(Scene &scene);

    private:
        GLuint rbo;
        GLuint framebuffer;
        Shader *shader;
};

#endif //__OPENGL_RENDERER_H__