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

struct IShader
{
    virtual ~IShader();
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

void triangle(Vec4f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer);

#endif //__OUR_GL_H__