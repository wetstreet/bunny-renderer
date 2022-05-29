#include "Mesh.h"
#include "common/Utils.h"

Mesh::Mesh(const char *path)
    : path(path)
{
    ParseFile();
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

void Mesh::ParseFile()
{
    std::unordered_map<glm::uvec3, unsigned int, uvec3_hash> vertMap;

    std::ifstream in;
    in.open(path, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            glm::vec3 v;
            for (int i=0;i<3;i++)
            {
                iss >> v[i];
            }
            verts.push_back(v);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            glm::vec2 v;
            for (int i=0;i<2;i++)
            {
                iss >> v[i];
            }
            uvs.push_back(v);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            glm::vec3 v;
            for (int i=0;i<3;i++)
            {
                iss >> v[i];
            }
            normals.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            glm::uvec3 tmp;
            iss >> trash;
            while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
                for (int i = 0; i < 3; i++)
                {
                    tmp[i]--;// in wavefront obj all indices start at 1, not zero
                }
                if (vertMap.count(tmp) == 0)
                {
                    Vertex vert{verts[tmp[0]], normals[tmp[2]] , uvs[tmp[1]]};
                    vertMap.insert(std::pair<glm::uvec3, unsigned int>(tmp, vertices.size()));
                    indices.push_back(vertices.size());
                    vertices.push_back(vert);
                }
                else
                {
                    indices.push_back(vertMap[tmp]);
                }
            }
        }
    }
}

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