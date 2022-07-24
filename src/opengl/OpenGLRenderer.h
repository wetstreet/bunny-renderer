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
        void GenerateIrradianceMap(Scene& scene);
        void GeneratePrefilterMap(Scene& scene);
        void GenerateLUT(Scene& scene);

    public:
	    GLuint postprocessRT;
        GLuint outlineRT;

    private:
        GLuint objectIdRT;
        GLuint rectVAO, rectVBO;

        GLuint rbo;
        GLuint FBO;

        GLuint postprocessFBO;
        GLuint outlineFBO;

        GLuint captureFBO, captureRBO;
        GLuint prefilterMap;
        GLuint brdfLUTTexture;

        unsigned int shadowMapWidth = 2048, shadowMapHeight = 2048;

        GLuint shadowMap;
        GLuint shadowMapFBO;

        std::shared_ptr<Shader> shadowMapShader;

        std::shared_ptr<Shader> postprocessShader;

        std::shared_ptr<Shader> outlineShader;
        std::shared_ptr<Shader> outlineCompareShader;
        std::shared_ptr<Shader> outlineBlurShader;
        std::shared_ptr<Shader> outlineMergeShader;
};

#endif //__OPENGL_RENDERER_H__