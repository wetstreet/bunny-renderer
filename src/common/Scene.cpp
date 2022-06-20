#include "Scene.h"

Scene::Scene(Camera &camera) : camera(camera)
{
	white_tex = std::make_shared<Texture>("res/obj/white_texture.png", GL_TEXTURE0);
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

void Scene::Draw()
{
    std::shared_ptr<Light> mainLight = GetMainLight();

    glm::vec3 lightPos;
    glm::vec3 lightColor;
    if (mainLight)
    {
        lightPos = mainLight->GetLightPosition();
        lightColor = mainLight->color * mainLight->intensity;
    }
    else
    {
        lightPos = -camera.Orientation;
        lightColor = glm::vec3(1);
    }

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
                // todo: move to material
                std::shared_ptr<Shader> shader = mesh->normalMap != NULL ? Shader::normalShader : Shader::defaultShader;

                shader->Activate();

                if (mesh->texture != NULL)
                {
                    mesh->texture->texUnit(*shader, "tex0", 0);
                    mesh->texture->Bind();
                }

                if (mesh->normalMap != NULL)
                {
                    mesh->normalMap->texUnit(*shader, "normalMap", 1);
                    mesh->normalMap->Bind();
                }

                glUniform4fv(glGetUniformLocation(shader->ID, "_Color"), 1, (float*)&mesh->color);

                glUniform1i(glGetUniformLocation(shader->ID, "_ObjectID"), i);
                glUniform3fv(glGetUniformLocation(shader->ID, "_MainLightPosition"), 1, (float*)&lightPos);
                glUniform3fv(glGetUniformLocation(shader->ID, "_MainLightColor"), 1, (float*)&lightColor);

                glUniformMatrix4fv(glGetUniformLocation(shader->ID, "br_ObjectToClip"), 1, GL_FALSE, glm::value_ptr(camera.cameraMatrix * mesh->objectToWorld));
                glUniformMatrix4fv(glGetUniformLocation(shader->ID, "br_ObjectToWorld"), 1, GL_FALSE, glm::value_ptr(mesh->objectToWorld));

                mesh->Draw();
            }
        }
    }
}


int Scene::AddPrimitive(std::string name)
{
	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(("res/obj/" + name + ".obj").c_str());
	mesh->texture = white_tex;
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