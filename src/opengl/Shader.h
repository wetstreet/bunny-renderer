#ifndef __SHADER_H__
#define __SHADER_H__

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>

std::string get_file_contents(const char* filename);

class Shader
{
    public:
        GLuint ID;
        Shader(const char* vertexFile, const char* fragmentFile);

        void Activate();
        void Delete();
    private:
        void compileErrors(unsigned int shader, const char* type);
};

#endif //__SHADER_H__