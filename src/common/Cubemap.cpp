#include "Cubemap.h"

Cubemap::Cubemap(std::string path)
{
	// All the faces of the cubemap (make sure they are in this exact order)
	std::string facesCubemap[6] =
	{
		"res/obj/skybox/right.jpg",
		"res/obj/skybox/left.jpg",
		"res/obj/skybox/top.jpg",
		"res/obj/skybox/bottom.jpg",
		"res/obj/skybox/front.jpg",
		"res/obj/skybox/back.jpg"
	};

	//glGenTextures(1, &ID);
	//glObjectLabel(GL_TEXTURE, ID, -1, "cubemap");
	//glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// This might help with seams on some systems
	//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	textures = new unsigned char* [6];
	stbi_set_flip_vertically_on_load(false);
	for (unsigned int i = 0; i < 6; i++)
	{
		textures[i] = stbi_load(facesCubemap[i].c_str(), &width, &height, &numColCh, 0);
		//glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textures[i]);
	}
}

Cubemap::~Cubemap()
{
	for (unsigned int i = 0; i < 6; i++)
		stbi_image_free(textures[i]);
	delete[] textures;
}