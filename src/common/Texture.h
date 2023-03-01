#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "stb_image.h"
#include "stb_image_write.h"
#include "Shader.h"

class Texture
{
    public:
        Texture(std::string path, GLenum type = GL_UNSIGNED_BYTE, GLenum wrap = GL_REPEAT, bool mipmap = true);
        ~Texture();

        glm::vec4 tex2D(glm::vec2& uv);
        void texUnit(Shader& shader, const char* uniform, GLuint unit);

        static void Init();
        static void Uninit();

    public:
        GLuint ID;
        std::string name;
        std::string path;

        unsigned char* bytes = nullptr;
        float* data = nullptr;
        int width, height, numColCh;

        bool mipmap = true;
        GLenum wrap = GL_REPEAT;
        GLenum type = GL_UNSIGNED_BYTE;

        static std::shared_ptr<Texture> white_tex;
        static std::shared_ptr<Texture> brick_tex;
        static std::shared_ptr<Texture> normal_tex;

    private:
        static glm::vec2 vec2_zero;
        static glm::vec2 vec2_one;
};

typedef void(*TexFunc)(Texture*);
extern TexFunc TextureRegisterFunction;
extern TexFunc TextureUnregisterFunction;

#endif //__TEXTURE_H__