#ifndef __VBO_H__
#define __VBO_H__

#include "glm/glm.hpp"
#include "glad/glad.h"
#include <vector>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

class VBO
{
    public:
        GLuint ID;
        VBO(std::vector<Vertex> &vertices);

        void Bind();
        void Unbind();
        void Delete();
};

#endif //__VBO_H__