#include "Mesh.h"

std::ostream &operator<<(std::ostream &out, glm::vec4 &v)
{
    out << "(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")";
    return out;
}

std::ostream &operator<<(std::ostream &out, glm::vec3 &v)
{
    out << "(" << v.x << "," << v.y << "," << v.z << ")";
    return out;
}

std::ostream &operator<<(std::ostream &out, glm::vec2 &v)
{
    out << "(" << v.x << "," << v.y << ")";
    return out;
}

std::ostream &operator<<(std::ostream &out, glm::mat4 &m)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            out << m[i][j] << " ";
        }
        out << std::endl;
    }
    return out;
}

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices)
{
    Mesh::vertices = vertices;
    Mesh::indices = indices;
    
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

Mesh::Mesh(const char *filename)
{
    std::vector<glm::vec3> verts;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;

    std::unordered_map<glm::uvec3, unsigned int, uvec3_hash> vertMap;
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    std::ifstream in;
    in.open (filename, std::ifstream::in);
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

    new (this)Mesh(vertices, indices);
}

static const float ZPI = 3.14159265358979323846f;
static const float RAD2DEG = (180.f / ZPI);
static const float DEG2RAD = (ZPI / 180.f);

static const glm::vec3 directionUnary[3] = { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) };

void Mesh::UpdateMatrix()
{
	objectToWorld = glm::mat4(1);

	for (int i = 0; i < 3; i++)
	{
		objectToWorld = glm::rotate(objectToWorld, rotation[i] * DEG2RAD, directionUnary[i]);
	}

	float validScale[3];
	for (int i = 0; i < 3; i++)
	{
		if (fabsf(scale[i]) < FLT_EPSILON)
		{
		    validScale[i] = 0.001f;
		}
		else
		{
		    validScale[i] = scale[i];
		}
	}
	objectToWorld[0] *= validScale[0];
	objectToWorld[1] *= validScale[1];
	objectToWorld[2] *= validScale[2];
	objectToWorld[3] = glm::vec4(position[0], position[1], position[2], 1.f);
}

void Mesh::Draw(Camera &camera)
{
    shader->Activate();

    vao.Bind();

    if (texture != NULL)
    {
        texture->texUnit(*shader, "tex0", 0);
        texture->Bind();
    }

    UpdateMatrix();

    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "camMatrix"), 1, GL_FALSE, glm::value_ptr(camera.cameraMatrix * objectToWorld));
    glm::vec3 lightDir = -camera.Orientation;
    glUniform3fv(glGetUniformLocation(shader->ID, "lightDir"), 1, (float*)&lightDir);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}