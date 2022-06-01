#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include "tgaimage.h"
#include "glm/glm.hpp"
#include "../common/Vertex.h"
#include "../common/Texture.h"
#include "../common/Camera.h"

glm::mat4 viewportMat(int x, int y, int w, int h);

struct Varying
{
    glm::vec4 position;
    glm::vec2 uv;
    glm::vec3 normal;
};

struct IShader
{
    virtual ~IShader();
    virtual Varying vertex(Vertex i) = 0;
    virtual glm::vec4 fragment(Varying &varying) = 0;
};

void triangle(Varying *varys, IShader &shader, uint8_t* pixels, float*zbuffer, int width, int height);

#endif //__OUR_GL_H__