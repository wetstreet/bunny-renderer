#ifndef __VERTEX_H__
#define __VERTEX_H__

#include "glm/glm.hpp"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
    glm::vec3 tangent;
};

struct Varying
{
    glm::vec4 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::mat3 TBN;
};

struct SkyboxVarying
{
    glm::vec4 position;
    glm::vec3 uv;
};

#endif // __VERTEX_H__