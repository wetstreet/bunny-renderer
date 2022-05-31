#ifndef __VBO_H__
#define __VBO_H__

#include <vector>
#include "glm/glm.hpp"
#include "glad/glad.h"
#include "../common/Vertex.h"

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