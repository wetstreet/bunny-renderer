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
    virtual void UnregisterTexture(Texture* texture) {};

    virtual void RegisterSkybox(Skybox* skybox) {};
    virtual void DrawSkybox() {};

    virtual void BindTexture(Texture& texture) {};

    virtual int GetObjectID(int x, int y) = 0;

    virtual void* GetRT() = 0;

public:
    GLuint postprocessRT;
    GLuint outlineRT;

    bool showIrradianceMap = false;
};

#endif //__REALTIME_RENDERER_H__