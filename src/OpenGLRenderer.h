#ifndef __OPENGL_RENDERER_H__
#define __OPENGL_RENDERER_H__

#include "common/Renderer.h"

class OpenGLRenderer : public Renderer
{
    public:
        OpenGLRenderer();
        virtual ~OpenGLRenderer();

        virtual void Render(Scene &scene);
	    GLuint postprocessRT;

    private:
        GLuint rbo;
        GLuint FBO;
        GLuint postprocessFBO;
	    GLuint rectVAO, rectVBO;
        Shader *shader;
        Shader *postprocessShader;
};

#endif //__OPENGL_RENDERER_H__