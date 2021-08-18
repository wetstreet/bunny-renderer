#include "Scene.h"

Scene::Scene(Camera *camera)
{
    Scene::camera = camera;
}

void Scene::Draw() const
{
    for (int i = 0; i < meshes.size(); i++)
    {
        Mesh *mesh = meshes[i];
        if (mesh->isEnabled)
        {
            mesh->Draw(*camera);
        }
    }
}

void Scene::AddMesh(Mesh *mesh)
{
    meshes.push_back(mesh);
}

void Scene::Delete()
{
    for (int i = 0; i < meshes.size(); i++)
    {
        delete meshes[i];
    }
}