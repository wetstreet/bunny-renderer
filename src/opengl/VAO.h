#ifndef __VAO_H__
#define __VAO_H__

#include "glad/glad.h"
#include "VBO.h"

class VAO
{
    public:
        GLuint ID;
        VAO();

        void LinkVBO(VBO vbo, GLuint layout);
        void Bind();
        void Unbind();
        void Delete();
};

#endif //__VAO_H__