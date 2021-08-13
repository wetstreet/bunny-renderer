#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), uvs_(), normals_(), faces_() {
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
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f v;
            for (int i=0;i<2;i++) iss >> v[i];
            int itrash;
            iss >> itrash;
            uvs_.push_back(v);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            normals_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<Vec3i> f;
            Vec3i tmp;
            iss >> trash;
            while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
                for (int i = 0; i < 3; i++)
                {
                    tmp[i]--;// in wavefront obj all indices start at 1, not zero
                }
                f.push_back(tmp);
            }
            faces_.push_back(f);
        }
    }
    // std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
    load_texture(filename, "_diffuse.tga", diffusemap_);
    load_texture(filename, "_nm_tangent.tga", normalmap_);
    load_texture(filename, "_spec.tga", specularmap_);
}

void Model::load_texture(std::string filename, const char *suffix, TGAImage &img)
{
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot != std::string::npos)
    {
        texfile = texfile.substr(0, dot) + std::string(suffix);
        // std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
        img.flip_vertically();
    }
}

TGAColor Model::diffuse(Vec2f uvf)
{
    Vec2i uv(uvf[0] * diffusemap_.get_width(), uvf[1] * diffusemap_.get_height());
    return diffusemap_.get(uv[0], uv[1]);
}

Vec3f Model::normalmap(Vec2f uvf)
{
    Vec2i uv(uvf[0] * normalmap_.get_width(), uvf[1] * normalmap_.get_height());
    TGAColor n = normalmap_.get(uv[0], uv[1]);
    Vec3f res;
    for (int i = 0; i < 3; i++)
    {
        res[2 - i] = (float)n[i] / 255.0f * 2.0f - 1.0f;
    }
    return res;
}

float Model::specular(Vec2f uvf)
{
    Vec2i uv(uvf[0] * specularmap_.get_width(), uvf[1] * specularmap_.get_height());
    return specularmap_.get(uv[0], uv[1])[0] / 1.0f;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nuvs() {
    return (int)uvs_.size();
}

int Model::nnormals() {
    return (int)normals_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec3f Model::vert(int iface, int nthvert) {
    return verts_[faces_[iface][nthvert][0]];
}

Vec2f Model::uv(int i) {
    return uvs_[i];
}

Vec2f Model::uv(int iface, int nthvert) {
    return uvs_[faces_[iface][nthvert][1]];
}

Vec3f Model::normal(int i) {
    return normals_[i];
}

Vec3f Model::normal(int iface, int nthvert) {
    return normals_[faces_[iface][nthvert][2]];
}

