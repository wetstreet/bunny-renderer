#ifndef __OPENGL_RENDERER_H__
#define __OPENGL_RENDERER_H__

#include "common/Renderer.h"

class OpenGLRenderer : public Renderer
{
    public:
        OpenGLRenderer();
        virtual ~OpenGLRenderer();

        int GetObjectID(int x, int y);
        virtual void Render(Scene &scene);
	    GLuint postprocessRT;
        GLuint outlineRT;

    private:
        GLuint objectIdRT;
        GLuint rbo;
        GLuint FBO;
        GLuint postprocessFBO;
        GLuint outlineFBO;
	    GLuint rectVAO, rectVBO;
        Shader* shader;
        Shader* postprocessShader;

        Shader* outlineShader;
        Shader* outlineCompareShader;
        Shader* outlineBlurShader;
        Shader* outlineMergeShader;
};

#endif //__OPENGL_RENDERER_H__