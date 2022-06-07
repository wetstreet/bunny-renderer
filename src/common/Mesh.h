#ifndef __MESH_H__
#define __MESH_H__

#include <string>
#include <unordered_map>

#include "../opengl/VAO.h"
#include "../opengl/EBO.h"
#include "Texture.h"
#include "Object.h"

class Mesh : public Object
{
    public:
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        std::vector<glm::vec3> verts;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> tangents;
        std::vector<glm::vec2> uvs;
        
        VAO vao;

        glm::vec3 minPos = glm::vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        glm::vec3 maxPos = glm::vec3(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

        glm::vec4 color = glm::vec4(1, 1, 1, 1);
        std::shared_ptr<Texture> texture;
        std::shared_ptr<Texture> normalMap;

        std::string path;

    public:
        Mesh(const char *filename);

        virtual Type GetType() { return Type_Mesh; };

        void Draw();

    private:
        void CalcBounds();
        void Bind();
        bool LoadMesh(const std::string& pFile);
};

#endif