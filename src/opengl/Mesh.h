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
        
        VAO vao;

        glm::vec3 position = glm::vec3(0);
        glm::vec3 rotation = glm::vec3(0);
        glm::vec3 scale = glm::vec3(1);

        Mesh(const char *filename);
        Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices);

        void Draw(Shader &shader, Camera &camera, Texture &texture);
};

#endif