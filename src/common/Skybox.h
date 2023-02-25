#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "Camera.h"

extern float skyboxVertices[24];
extern unsigned int skyboxIndices[36];

class Skybox
{
    public:
        Skybox();
        ~Skybox();
        glm::vec4 texCube(glm::vec3 direction);
        glm::vec4 texCube_f(glm::vec3 direction);

        unsigned char** textures;
        float** textures_f;
        int width, height, nrChannels;
};

typedef void(*SkyboxFunc)(Skybox*);
extern SkyboxFunc SkyboxRegisterFunction;
extern SkyboxFunc SkyboxUnregisterFunction;

#endif // __SKYBOX_H__