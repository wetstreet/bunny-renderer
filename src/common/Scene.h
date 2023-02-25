#ifndef __SCENE_H__
#define __SCENE_H__

#include "Mesh.h"
#include "Texture.h"
#include "Light.h"
#include "Camera.h"
#include "Skybox.h"

class Scene
{
    public:
        Camera &camera;
        glm::vec3 ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
        std::vector<std::shared_ptr<Object>> objects;

        Skybox skybox;

        GLuint shadowMap;

        std::shared_ptr<Texture> equirectangular;

    public:
        Scene(Camera &camera);
        ~Scene();

        void Scene::LoadScene(const std::string& pFile);

        std::shared_ptr<Light> GetMainLight();
        void GetMainLightProperties(glm::vec3& dir, glm::vec3& color);
        glm::mat4 GetLightMatrix();

        void UpdateMatrices();

        int AddObject(std::shared_ptr<Object>);
        void RemoveObject(int index);
        int AddPrimitive(std::string name);
};

#endif //__SCENE_H__