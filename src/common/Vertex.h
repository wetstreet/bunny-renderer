#ifndef __VERTEX_H__
#define __VERTEX_H__

#include "glm/glm.hpp"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

#endif // __VERTEX_H__