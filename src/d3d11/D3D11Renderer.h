#ifndef __D3D11_RENDERER_H__
#define __D3D11_RENDERER_H__

#include "common/RealtimeRenderer.h"

class D3D11Renderer : public RealtimeRenderer
{
public:
    D3D11Renderer();
    virtual ~D3D11Renderer();

    virtual int GetObjectID(int x, int y);
    virtual void Render(Scene& scene);

    //virtual void GenerateCubemapFromEquirectangular(Scene& scene);
    //virtual void GenerateIrradianceMap(Scene& scene);
    //virtual void GeneratePrefilterMap(Scene& scene);
    //virtual void GenerateBrdfLUT(Scene& scene);

    virtual void D3D11Renderer::RegisterTexture(unsigned int ID, const char* name, int width, int height, GLenum format, GLenum type, GLenum wrap, bool mipmap, unsigned char* bytes);
    virtual void D3D11Renderer::RegisterTextureF(unsigned int ID, const char* name, int width, int height, GLenum format, GLenum type, GLenum wrap, bool mipmap, float* data);

    virtual void BindTexture(Texture& texture);

    virtual void* GetRT() { return rt; };
public:
    GLuint postprocessRT;
    GLuint outlineRT;

    void* rt;

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

#endif //__D3D11_RENDERER_H__