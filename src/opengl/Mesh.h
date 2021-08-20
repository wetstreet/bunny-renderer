#ifndef __MESH_H__
#define __MESH_H__

#include <string>
#include <unordered_map>

#include "VAO.h"
#include "EBO.h"
#include "Camera.h"
#include "Texture.h"
#include "../common/Object.h"

class Mesh : public Object
{
    public:
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        
        VAO vao;

        virtual Type GetType() { return Type_Mesh; };

        Texture *texture;
        Shader *shader;

        Mesh(const char *filename);
        Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices);

        void Draw(Camera &camera, glm::vec3 &lightPos, glm::vec3 &lightColor);
};

std::ostream &operator<<(std::ostream &out, glm::vec4 &v);
std::ostream &operator<<(std::ostream &out, glm::vec3 &v);
std::ostream &operator<<(std::ostream &out, glm::vec2 &v);
std::ostream &operator<<(std::ostream &out, glm::mat4 &m);

#endif