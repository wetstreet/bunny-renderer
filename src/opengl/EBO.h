#ifndef __EBO_H__
#define __EBO_H__

#include "glad/glad.h"

class EBO
{
    public:
        GLuint ID;
        EBO(GLuint *indices, GLsizeiptr size);

        void Bind();
        void Unbind();
        void Delete();
};

#endif //__EBO_H__