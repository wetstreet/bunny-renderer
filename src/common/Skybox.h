#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"

class Skybox
{
    public:
        Skybox();
        ~Skybox();
        void Draw(Camera &camera);

    private:
        Shader *skyboxShader;
        unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
        unsigned int cubemapTexture;
};

#endif // __SKYBOX_H__