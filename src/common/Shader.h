#ifndef __SHADER_H__
#define __SHADER_H__

#include "common/Utils.h"
#include <glad/glad.h>

class Shader
{
    public:
        GLuint ID;
        Shader(std::string shaderName);
        ~Shader();

        static void Init();
        static std::shared_ptr<Shader> unlitShader;
        static std::shared_ptr<Shader> defaultShader;
        static std::shared_ptr<Shader> normalShader;

        void Activate();
};

#endif //__SHADER_H__