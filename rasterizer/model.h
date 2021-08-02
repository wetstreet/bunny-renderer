#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec2f> uvs_;
	std::vector<Vec3f> normals_;
	std::vector<std::vector<Vec3i> > faces_;
    TGAImage diffusemap_;
    TGAImage normalmap_;
    TGAImage specularmap_;
	void load_texture(std::string filename, const char *suffix, TGAImage &img);
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nuvs();
	int nnormals();
	int nfaces();
	TGAColor diffuse(Vec2f uv);
	Vec3f normalmap(Vec2f uv);
	float specular(Vec2f uv);
	Vec3f vert(int i);
	Vec3f vert(int iface, int nthvert);
	Vec2f uv(int i);
	Vec2f uv(int iface, int nthvert);
	Vec3f normal(int i);
	Vec3f normal(int iface, int nthvert);
	std::vector<Vec3i> face(int idx);
};

#endif //__MODEL_H__
