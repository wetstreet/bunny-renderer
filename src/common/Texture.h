#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "stb_image.h"

#include "Shader.h"

class Texture
{
    public:
        GLuint ID;
        std::string name;
        std::string path;

        unsigned char* bytes;
        int width, height, numColCh;
        Texture(const char* image);
        ~Texture();

        glm::vec4 tex2D(glm::vec2& uv);
        void texUnit(Shader& shader, const char* uniform, GLuint unit);
        void Bind();
        void Unbind();
    private:
        static glm::vec2 vec2_zero;
        static glm::vec2 vec2_one;
};

#endif //__TEXTURE_H__