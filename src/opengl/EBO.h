#ifndef __EBO_H__
#define __EBO_H__

#include "glad/glad.h"
#include <vector>

class EBO
{
    public:
        GLuint ID;
        EBO(std::vector<GLuint> &indices);

        void Bind();
        void Unbind();
        void Delete();
};

#endif //__EBO_H__