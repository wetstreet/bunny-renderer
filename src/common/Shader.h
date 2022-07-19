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

        void ReloadShader();
        void Activate();

        static void Init();
        static std::shared_ptr<Shader> unlitShader;
        static std::shared_ptr<Shader> defaultShader;
        static std::shared_ptr<Shader> normalShader;
    private:
        void LoadShader();
        std::string shaderName;
};

#endif //__SHADER_H__