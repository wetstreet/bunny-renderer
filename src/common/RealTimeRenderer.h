#ifndef __REALTIME_RENDERER_H__
#define __REALTIME_RENDERER_H__

#include "Renderer.h"

class RealtimeRenderer : public Renderer
{
public:
    virtual ~RealtimeRenderer() {}

    virtual void GenerateCubemapFromEquirectangular(Scene& scene) {};
    virtual void GenerateIrradianceMap(Scene& scene) {};
    virtual void GeneratePrefilterMap(Scene& scene) {};
    virtual void GenerateBrdfLUT(Scene& scene) {};

    virtual void RegisterTexture(Texture* texture) {};
    virtual void RegisterMesh(Mesh* mesh) {};
    virtual void RegisterSkybox(Skybox* skybox) {};

    virtual void UnregisterTexture(Texture* texture) {};
    virtual void UnregisterMesh(Mesh* mesh) {};
    virtual void UnregisterSkybox(Skybox* skybox) {};

    virtual void BindSkybox(Skybox* skybox, GLuint slot) {};
    virtual void BindTexture(Texture& texture, GLuint slot) {};

    virtual void DrawSkybox() {};
    virtual void DrawScene(Scene& scene) {};
    virtual void DrawMesh(Mesh& mesh) {};

    virtual int GetObjectID(int x, int y) = 0;

    virtual void* GetRT() = 0;

    // for imgui::image
    virtual ImVec2& GetUV0() { return uv0; }
    virtual ImVec2& GetUV1() { return uv1; }

    // dx
    ImVec2 uv0{ 0, 0 };
    ImVec2 uv1{ 1, 1 };

    // opengl
    ImVec2 uv0_flip{ 0, 1 };
    ImVec2 uv1_flip{ 1, 0 };

public:
    GLuint postprocessRT;
    GLuint outlineRT;

    bool showIrradianceMap = false;
};

#endif //__REALTIME_RENDERER_H__