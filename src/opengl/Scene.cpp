#include "Scene.h"

Scene::Scene(Camera *camera)
{
    Scene::camera = camera;
    
	head_diffuse = new Texture("obj/african_head/african_head_diffuse.tga", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
	white_tex = new Texture("obj/white_texture.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);

	Mesh *head = new Mesh("obj/african_head/african_head.obj");
	head->texture = head_diffuse;
    strcpy(head->name, "head");
	AddObject(head);
}

Scene::~Scene()
{
    for (int i = 0; i < objects.size(); i++)
    {
        delete objects[i];
    }
    delete head_diffuse;
    delete white_tex;
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

void Scene::Draw(Shader *shader)
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
            mesh->Draw(*camera, lightPos, lightColor, shader);
        }
    }
}

int Scene::AddPrimitive(std::string name)
{
	Mesh *mesh = new Mesh(("obj/" + name + ".obj").c_str());
	mesh->texture = white_tex;
    strcpy(mesh->name, name.c_str());
	return AddObject(mesh);
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