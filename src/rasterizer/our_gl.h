#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include "tgaimage.h"
#include "glm/glm.hpp"
#include "../opengl/VBO.h"
#include "../opengl/Texture.h"

glm::mat4 model_matrix(glm::vec3 &position, glm::vec3 &scale);
glm::mat4 viewport(int x, int y, int w, int h);
glm::mat4 projection(float coeff = 0.0f); // coeff = -1/c
glm::mat4 lookat(glm::vec3 eye, glm::vec3 center, glm::vec3 up);

struct Varying
{
    glm::vec4 position;
    glm::vec2 uv;
    glm::vec3 normal;
};

struct IShader
{
    virtual ~IShader();
    virtual Varying vertex(Vertex i, glm::mat4 MVP) = 0;
    virtual glm::vec4 fragment(glm::vec2 uv, Texture *tex) = 0;
};

void triangle(Varying *varys, IShader &shader, uint8_t* pixels, int *zbuffer, int width, int height, Texture *texture);

#endif //__OUR_GL_H__