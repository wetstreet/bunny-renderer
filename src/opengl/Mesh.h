#ifndef __MESH_H__
#define __MESH_H__

#include <string>

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

        Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices);

        void Draw(Shader &shader, Camera &camera, Texture &texture);
};

// Model::Model(const char *filename)
// {
//     std::ifstream in;
//     in.open (filename, std::ifstream::in);
//     if (in.fail()) return;
//     std::string line;
//     while (!in.eof()) {
//         std::getline(in, line);
//         std::istringstream iss(line.c_str());
//         char trash;
//         if (!line.compare(0, 2, "v ")) {
//             iss >> trash;
//             float v;
//             for (int i=0;i<3;i++)
//             {
//                 iss >> v;
//                 verts.push_back(v);
//             }
//         } else if (!line.compare(0, 3, "vt ")) {
//             iss >> trash >> trash;
//             float v;
//             for (int i=0;i<2;i++)
//             {
//                 iss >> v;
//                 uvs.push_back(v);
//             }
//         } else if (!line.compare(0, 3, "vn ")) {
//             iss >> trash >> trash;
//             float v;
//             for (int i=0;i<3;i++)
//             {
//                 iss >> v;
//                 normals.push_back(v);
//             }
//         } else if (!line.compare(0, 2, "f ")) {
//             std::vector<Vec3i> f;
//             Vec3i tmp;
//             iss >> trash;
//             while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
//                 for (int i = 0; i < 3; i++)
//                 {
//                     tmp[i]--;// in wavefront obj all indices start at 1, not zero
//                 }
//                 f.push_back(tmp);
//             }
//             faces_.push_back(f);
//         }
//     }
//     std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
// }

#endif