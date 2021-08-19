#include "Scene.h"

Scene::Scene(Camera *camera)
{
    Scene::camera = camera;
}

Light *Scene::GetMainLight()
{
    Light *mainLight = NULL;
    for (int i = 0; i < objects.size(); i++)
    {
        Object *object = objects[i];
        if (object->GetType() == Type_Light && object->isEnabled)
        {
            Light *light = (Light*)object;
            if (mainLight == NULL || light->intensity > mainLight->intensity)
                mainLight = light;
        }
    }
    
    return mainLight;
}

void Scene::Draw()
{
    Light *mainLight = GetMainLight();

    glm::vec3 lightPos;
    glm::vec3 lightColor;
    if (mainLight)
    {
        lightPos = mainLight->GetLightPosition();
        lightColor = mainLight->color * mainLight->intensity;
    }
    else
    {
        lightPos = -camera->Orientation;
        lightColor = glm::vec3(1);
    }

    for (int i = 0; i < objects.size(); i++)
    {
        Object *object = objects[i];
        if (object->GetType() == Type_Mesh && object->isEnabled)
        {
            Mesh *mesh = (Mesh*)object;
            mesh->Draw(*camera, lightPos, lightColor);
        }
    }
}

int Scene::AddObject(Object *object)
{
    objects.push_back(object);
    return objects.size() - 1;
}

void Scene::RemoveObject(int index)
{
    Object *object = objects[index];
    objects.erase(objects.begin() + index);
    delete object;
}

Scene::~Scene()
{
    for (int i = 0; i < objects.size(); i++)
    {
        delete objects[i];
    }
}