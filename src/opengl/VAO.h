#ifndef __VAO_H__
#define __VAO_H__

#include "glad/glad.h"
#include "VBO.h"

class VAO
{
    public:
        GLuint ID;
        VAO();

        void LinkAttrib(VBO vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
        void Bind();
        void Unbind();
        void Delete();
};

#endif //__VAO_H__