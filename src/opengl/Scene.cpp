#include "Scene.h"

Scene::Scene(Camera *camera)
{
    Scene::camera = camera;
}

Light *Scene::GetMainLight()
{
    if (lights.size() == 0) return NULL;

    Light *mainLight = lights[0];
    for (int i = 1; i < lights.size(); i++)
    {
        if (lights[i]->intensity > mainLight->intensity)
            mainLight = lights[i];
    }
    
    return mainLight;
}

void Scene::Draw()
{
    Light *mainLight = GetMainLight();
    for (int i = 0; i < meshes.size(); i++)
    {
        Mesh *mesh = meshes[i];
        if (mesh->isEnabled)
        {
            mesh->Draw(*camera, mainLight);
        }
    }
}

int Scene::AddMesh(Mesh *mesh)
{
    meshes.push_back(mesh);
    return meshes.size() - 1;
}

void Scene::RemoveMesh(int index)
{
    Mesh *mesh = meshes[index];
    meshes.erase(meshes.begin() + index);
    delete mesh;
}

void Scene::Delete()
{
    for (int i = 0; i < meshes.size(); i++)
    {
        delete meshes[i];
    }
    for (int i = 0; i < lights.size(); i++)
    {
        delete lights[i];
    }
}