#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "Camera.h"

extern float skyboxVertices[];
extern unsigned int skyboxIndices[];

class Skybox
{
    public:
        Skybox();
        ~Skybox();
        void Bind(GLuint slot = 0);
        void DrawMesh();
        void Draw(Camera &camera);
        glm::vec4 texCube(glm::vec3 direction);

        bool showIrradianceMap = false;
        unsigned int irradianceMap;
        unsigned int prefilterMap;
        unsigned int brdfLUT; // put it here temporarily

        unsigned char** textures;
        int width, height, nrChannels;

    private:
        unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
        unsigned int cubemapTexture;
};

#endif // __SKYBOX_H__