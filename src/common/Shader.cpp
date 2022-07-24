#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>

#ifdef __DEVELOPMENT__
std::string RESOURCE_PATH("D:/bunny-renderer/"); // local resource path
#else
std::string RESOURCE_PATH(""); // relative resource path
#endif

std::shared_ptr<Shader> Shader::unlitShader;
std::shared_ptr<Shader> Shader::defaultShader;
std::shared_ptr<Shader> Shader::normalShader;
std::shared_ptr<Shader> Shader::skyboxShader;

void Shader::Init()
{
    unlitShader = std::make_shared<Shader>("unlit");
    defaultShader = std::make_shared<Shader>("default");
    normalShader = std::make_shared<Shader>("normal");
    skyboxShader = std::make_shared<Shader>("skybox");
}

void CompileErrors(unsigned int shader, const char* type, std::string path)
{
    GLint hasCompiled;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_COMPILAATION_ERROR for: " << type << std::endl
                << path << std::endl
                << infoLog << "\n" << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_LINKING_ERROR for:" << type << std::endl
                << path << std::endl
                << infoLog << "\n" << std::endl;
        }
    }
}

void Shader::LoadShader()
{
    std::string vertexCode = GetFileContents(RESOURCE_PATH + "res/shaders/" + shaderName + ".vert");
    std::string fragmentCode = GetFileContents(RESOURCE_PATH + "res/shaders/" + shaderName + ".frag");

    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    CompileErrors(vertexShader, "VERTEX", shaderName);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    CompileErrors(fragmentShader, "FRAGMENT", shaderName);

    ID = glCreateProgram();
    glObjectLabel(GL_PROGRAM, ID, -1, shaderName.c_str());
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);
    CompileErrors(ID, "PROGRAM", shaderName);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::Shader(std::string shaderName) : shaderName(shaderName)
{
    LoadShader();
}

void Shader::SetUniform(const GLchar* name, float f)
{
    glUniform1f(glGetUniformLocation(ID, name), f);
}
void Shader::SetUniform(const GLchar* name, int i)
{
    glUniform1i(glGetUniformLocation(ID, name), i);
}
void Shader::SetUniform(const GLchar* name, glm::vec3& v)
{
    glUniform3fv(glGetUniformLocation(ID, name), 1, (float*)&v);
}
void Shader::SetUniform(const GLchar* name, glm::vec4& v)
{
    glUniform3fv(glGetUniformLocation(ID, name), 1, (float*)&v);
}
void Shader::SetUniform(const GLchar* name, glm::mat4& m)
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::Activate()
{
    glUseProgram(ID);
}

void Shader::ReloadShader()
{
    glDeleteProgram(ID);
    LoadShader();
}

Shader::~Shader()
{
    glDeleteProgram(ID);
}