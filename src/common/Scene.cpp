#include "Scene.h"

Scene::Scene(Camera &camera) : camera(camera)
{
}

Scene::~Scene()
{
    objects.clear();
}

std::shared_ptr<Light> Scene::GetMainLight()
{
    std::shared_ptr<Light> mainLight = NULL;
    for (int i = 0; i < objects.size(); i++)
    {
        if (objects[i]->GetType() == Type_Light && objects[i]->isEnabled)
        {
            std::shared_ptr<Light> light = std::dynamic_pointer_cast<Light>(objects[i]);
            if (mainLight == NULL || light->intensity > mainLight->intensity)
                mainLight = light;
        }
    }
    
    return mainLight;
}

void Scene::GetMainLightProperties(glm::vec3& dir, glm::vec3& color)
{
    std::shared_ptr<Light> mainLight = GetMainLight();
    if (mainLight)
    {
        dir = mainLight->GetLightPosition();
        color = mainLight->color * mainLight->intensity;
    }
    else
    {
        dir = -camera.Orientation;
        color = glm::vec3(1);
    }

}

void Scene::Draw()
{
    glm::vec3 lightPos;
    glm::vec3 lightColor;
    GetMainLightProperties(lightPos, lightColor);

    for (int i = 0; i < objects.size(); i++)
    {
        std::shared_ptr<Object> object = objects[i];
        if (object->GetType() == Type_Mesh)
        {
            std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(object);

            // draw outline when invisible, so always needs to update transform matrix
            mesh->UpdateMatrix();

            if (mesh->isEnabled)
            {
                // per material setup
                mesh->material->Setup();

                // common setup
                mesh->material->SetUniform("_ObjectID", i);
                mesh->material->SetUniform("_MainLightPosition", lightPos);
                mesh->material->SetUniform("_MainLightColor", lightColor);
                mesh->material->SetUniform("br_ObjectToClip", camera.cameraMatrix * mesh->objectToWorld);
                mesh->material->SetUniform("br_ObjectToWorld", mesh->objectToWorld);
                mesh->material->SetUniform("br_WorldToObject", mesh->worldToObject);

                mesh->Draw();
            }
        }
    }
}

int Scene::AddPrimitive(std::string name)
{
	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(("res/obj/" + name + ".obj").c_str());
    strcpy_s(mesh->name, 32, name.c_str());
	return AddObject(mesh);
}

int Scene::AddObject(std::shared_ptr<Object> object)
{
    objects.push_back(object);
    return (int)objects.size() - 1;
}

void Scene::RemoveObject(int index)
{
    std::shared_ptr<Object> object = objects[index];
    objects.erase(objects.begin() + index);
}