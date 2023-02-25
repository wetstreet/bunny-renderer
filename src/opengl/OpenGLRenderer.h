#ifndef __OPENGL_RENDERER_H__
#define __OPENGL_RENDERER_H__

#include <unordered_map>

#include "common/RealtimeRenderer.h"
#include "VBO.h"
#include "EBO.h"

class OpenGLRenderer : public RealtimeRenderer
{
    public:
        OpenGLRenderer();
        virtual ~OpenGLRenderer();

        int GetObjectID(int x, int y);
        virtual void Render(Scene &scene);

        virtual void GenerateCubemapFromEquirectangular(Scene& scene);
        virtual void GenerateIrradianceMap(Scene& scene);
        virtual void GeneratePrefilterMap(Scene& scene);
        virtual void GenerateBrdfLUT(Scene& scene);

        virtual void RegisterTexture(Texture* texture);
        virtual void RegisterMesh(Mesh* mesh);
        virtual void RegisterSkybox(Skybox* skybox);

        virtual void UnregisterTexture(Texture* texture);
        virtual void UnregisterMesh(Mesh* mesh);
        virtual void UnregisterSkybox(Skybox* skybox);

        virtual void BindSkybox(Skybox* skybox, GLuint slot);
        virtual void BindTexture(Texture& texture, GLuint slot);

        virtual void DrawSkybox();
        virtual void DrawScene(Scene& scene);
        virtual void DrawMesh(Mesh& mesh);

        virtual void* GetRT() { return (void*)(intptr_t)renderTexture; };
    public:

    private:
        GLuint objectIdRT;
        GLuint rectVAO, rectVBO;

        GLuint rbo;
        GLuint FBO;

        GLuint postprocessFBO;
        GLuint outlineFBO;

        GLuint captureFBO, captureRBO;
        GLuint envCubemap;
        GLuint irradianceMap;
        GLuint prefilterMap;
        GLuint brdfLUTTexture;

        unsigned int skyboxVAO;
        unsigned int cubemapTexture;

        unsigned int shadowMapWidth = 2048, shadowMapHeight = 2048;

        GLuint shadowMap;
        GLuint shadowMapFBO;

        std::shared_ptr<Shader> equirectangularShader;
        std::shared_ptr<Shader> irradianceShader;
        std::shared_ptr<Shader> prefilterShader;

        std::shared_ptr<Shader> shadowMapShader;

        std::shared_ptr<Shader> postprocessShader;

        std::shared_ptr<Shader> outlineShader;
        std::shared_ptr<Shader> outlineCompareShader;
        std::shared_ptr<Shader> outlineBlurShader;
        std::shared_ptr<Shader> outlineMergeShader;
};

#endif //__OPENGL_RENDERER_H__