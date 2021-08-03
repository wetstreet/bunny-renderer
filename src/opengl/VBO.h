#ifndef __VBO_H__
#define __VBO_H__

#include "glad/glad.h"

class VBO
{
    public:
        GLuint ID;
        VBO(GLfloat *vertices, GLsizeiptr size);

        void Bind();
        void Unbind();
        void Delete();
};

#endif //__VBO_H__