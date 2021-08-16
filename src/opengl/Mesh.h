#ifndef __MESH_H__
#define __MESH_H__

#include <string>
#include <unordered_map>

#include "VAO.h"
#include "EBO.h"
#include "Camera.h"
#include "Texture.h"

class Mesh
{
    public:
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        
        std::string name;
        VAO vao;

        glm::vec3 position = glm::vec3(0);
        glm::vec3 rotation = glm::vec3(0);
        glm::vec3 scale = glm::vec3(1);
        glm::mat4 objectToWorld = glm::mat4(1);

        Texture *texture;
        Shader *shader;

        Mesh(const char *filename);
        Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices);

        void UpdateMatrix();
        void Draw(Camera &camera);
};

std::ostream &operator<<(std::ostream &out, glm::vec4 &v);
std::ostream &operator<<(std::ostream &out, glm::vec3 &v);
std::ostream &operator<<(std::ostream &out, glm::vec2 &v);
std::ostream &operator<<(std::ostream &out, glm::mat4 &m);

#endif