#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include "tgaimage.h"
#include "glm/glm.hpp"
#include "../common/Vertex.h"
#include "../common/Texture.h"
#include "../common/Camera.h"

glm::mat4 model_matrix(glm::vec3 &position, glm::vec3 &rotation, glm::vec3 &scale);
glm::mat4 viewportMat(int x, int y, int w, int h);
glm::mat4 projection(float coeff = 0.0f); // coeff = -1/c
glm::mat4 lookat(glm::vec3 &eye, glm::vec3 &center, glm::vec3 &up);

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

glm::vec3 tex2D(Texture &tex, glm::vec2 &uv);
void triangle(Varying *varys, IShader &shader, uint8_t* pixels, float*zbuffer, int width, int height, Camera& camera);

#endif //__OUR_GL_H__