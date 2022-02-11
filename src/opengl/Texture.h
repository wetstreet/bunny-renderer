#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "glad/glad.h"
#include "stb_image.h"

#include "Shader.h"

class Texture
{
    public:
        GLuint ID;
        GLenum type;
        unsigned char* bytes;
        int width, height, numColCh;
        Texture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType);
        ~Texture();

        void texUnit(Shader& shader, const char* uniform, GLuint unit);
        void Bind();
        void Unbind();
};

#endif //__TEXTURE_H__