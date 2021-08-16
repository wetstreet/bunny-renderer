#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include "tgaimage.h"
#include "geometry.h"
#include "glm/glm.hpp"

extern Matrix ModelView;
extern Matrix Projection;
extern Matrix Viewport;

void viewport(int x, int y, int w, int h);
void projection(float coeff = 0.0f); // coeff = -1/c
void lookat(glm::vec3 eye, glm::vec3 center, glm::vec3 up);

glm::mat4 viewport_ext(int x, int y, int w, int h);
glm::mat4 projection_ext(float coeff = 0.0f); // coeff = -1/c
glm::mat4 lookat_ext(glm::vec3 eye, glm::vec3 center, glm::vec3 up);

struct IShader
{
    virtual ~IShader();
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

struct Varying
{
    glm::vec4 position;
    glm::vec2 uv;
    glm::vec3 normal;
};

void triangle(Vec4f *pts, IShader &shader, uint8_t* pixels, int *zbuffer, int width, int height);
void triangle_ext(Varying *varys, IShader &shader, uint8_t* pixels, int *zbuffer, int width, int height);

#endif //__OUR_GL_H__