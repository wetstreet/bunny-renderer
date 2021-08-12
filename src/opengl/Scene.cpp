#include "Scene.h"

Scene::Scene(Camera *camera)
{
    Scene::camera = camera;
}

void Scene::Draw() const
{
    for (int i = 0; i < meshes.size(); i++)
    {
        Mesh mesh = *meshes[i];
        mesh.Draw(*camera);
    }
}

void Scene::AddMesh(Mesh &mesh)
{
    meshes.push_back(&mesh);
}