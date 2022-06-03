#include "Mesh.h"
#include "common/Utils.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

bool Mesh::LoadMesh(const std::string& pFile) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(pFile,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices);

    if (nullptr == scene) {
        std::cout << "Mesh load failed!: " << pFile << std::endl  << importer.GetErrorString() << std::endl;
        return false;
    }

    const aiMesh* model = scene->mMeshes[0];

    const aiVector3D aiZeroVector(0.0f, 0.0f, 0.0f);
    for (unsigned int i = 0; i < model->mNumVertices; i++)
    {
        const aiVector3D* pPos = &(model->mVertices[i]);
        const aiVector3D* pNormal = &(model->mNormals[i]);
        const aiVector3D* pTexCoord = model->HasTextureCoords(0) ? &(model->mTextureCoords[0][i]) : &aiZeroVector;

        Vertex vert{ glm::vec3(pPos->x, pPos->y, pPos->z),
                    glm::vec3(pNormal->x, pNormal->y, pNormal->z),
                    glm::vec2(pTexCoord->x, pTexCoord->y)};

        vertices.push_back(vert);
    }

    for (unsigned int i = 0; i < model->mNumFaces; i++)
    {
        const aiFace& face = model->mFaces[i];
        assert(face.mNumIndices == 3);
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }

    return true;
}

Mesh::Mesh(const char *path)
    : path(path)
{
    LoadMesh(path);
    CalcBounds();
    Bind();
}

inline std::size_t hash_combine(const std::size_t& seed1, const std::size_t& seed2)
{
    return seed1 ^ (seed2 + 0x9e3779b9 + (seed1<<6) + (seed1>>2));
}

struct uvec3_hash {
    std::size_t operator () (const glm::uvec3 &v) const {
        auto h1 = std::hash<unsigned int>{}(v.x);
        auto h2 = std::hash<unsigned int>{}(v.y);
        auto h3 = std::hash<unsigned int>{}(v.z);

        std::size_t h12 = hash_combine(h1, h2);
        return hash_combine(h12, h3);
    }
};

void Mesh::CalcBounds()
{
    for (int i = 0; i < verts.size(); i++)
    {
        glm::vec3 pos = verts[i];
        if (pos.x < minPos.x) minPos.x = pos.x;
        if (pos.y < minPos.y) minPos.y = pos.y;
        if (pos.z < minPos.z) minPos.z = pos.z;

        if (pos.x > maxPos.x) maxPos.x = pos.x;
        if (pos.y > maxPos.y) maxPos.y = pos.y;
        if (pos.z > maxPos.z) maxPos.z = pos.z;
    }
}

void Mesh::Bind()
{
	vao.Bind();

	VBO vbo(vertices);
	EBO ebo(indices);

	vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), 0);
	vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	vao.LinkAttrib(vbo, 2, 2, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}

void Mesh::Draw()
{
    vao.Bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}