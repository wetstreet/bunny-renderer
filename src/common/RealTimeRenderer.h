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

    virtual void RegisterTexture(unsigned int ID, const char* name, int width, int height, GLenum format, GLenum type, GLenum wrap, bool mipmap, unsigned char* bytes) {};
    virtual void RegisterTextureF(unsigned int ID, const char* name, int width, int height, GLenum format, GLenum type, GLenum wrap, bool mipmap, float* data) {};

    virtual void BindTexture(Texture& texture) {};

    virtual int GetObjectID(int x, int y) = 0;

    virtual void* GetRT() = 0;

public:
    GLuint postprocessRT;
    GLuint outlineRT;
};

#endif //__REALTIME_RENDERER_H__