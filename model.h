#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> uvs_;
	std::vector<Vec3f> normals_;
	std::vector<std::vector<int> > faces_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nuvs();
	int nnormals();
	int nfaces();
	Vec3f vert(int i);
	Vec3f uv(int i);
	Vec3f normal(int i);
	std::vector<int> face(int idx);
};

#endif //__MODEL_H__
