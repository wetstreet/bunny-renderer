#ifndef __MESH_H__
#define __MESH_H__

#include <string>
#include <unordered_map>

#include "../opengl/VAO.h"
#include "../opengl/EBO.h"
#include "Texture.h"
#include "Object.h"
#include "Material.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

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

        std::string path;

        std::shared_ptr<Material> material;

    public:
        Mesh(const char *filename);
        Mesh(const aiMesh* model);

        virtual Type GetType() { return Type_Mesh; };

        void Draw();

    private:
        void CalcBounds();
        void Bind();
        void InitMesh(const aiMesh* model);
        bool LoadMesh(const std::string& pFile);
};

#endif