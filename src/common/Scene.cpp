#include "Scene.h"
#include "DirectionalLight.h"
#include <map>

Scene::Scene(Camera &camera) : camera(camera)
{
    AddObject(std::make_shared<DirectionalLight>());
}

Scene::~Scene()
{
    objects.clear();
}

std::string getBasePath(const std::string& path)
{
    size_t pos = path.find_last_of("\\/");
    return (std::string::npos == pos) ? "" : path.substr(0, pos + 1);
}

void Scene::LoadScene(const std::string& pFile)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(pFile,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices);

    if (nullptr == scene) {
        std::cout << "Scene load failed!: " << pFile << std::endl << importer.GetErrorString() << std::endl;
        return;
    }
    std::cout << "Loading Scene: " << pFile << std::endl;

    aiVector3D position, rotation, scale;
    scene->mRootNode->mTransformation.Decompose(scale, rotation, position);

    std::string folder = getBasePath(pFile);

    std::map<std::string, std::shared_ptr<Texture>> textureMap;
    std::vector<std::shared_ptr<Material>> materials;

    std::cout << "mNumMaterials=" << scene->mNumMaterials << std::endl;
    for (int i = 0; i < scene->mNumMaterials; i++)
    {
        aiMaterial* aimaterial = scene->mMaterials[i];

        auto mat = std::make_shared<NormalMaterial>();

        aiString path;
        aiColor4D diffuse;

        if (AI_SUCCESS == aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path))
        {
            mat->texture = std::make_shared<Texture>((folder + path.data).c_str());
        }

        if (AI_SUCCESS == aimaterial->GetTexture(aiTextureType_NORMALS, 0, &path))
        {
            mat->normalMap = std::make_shared<Texture>((folder + path.data).c_str());
        }

        if (AI_SUCCESS == aiGetMaterialColor(aimaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
        {
            mat->color = glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
        }
        materials.push_back(mat);
    }

    std::cout << "mNumMeshes=" << scene->mNumMeshes << std::endl;
    for (int i = 0; i < scene->mNumMeshes; i++)
    {
        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(scene->mMeshes[i]);
        mesh->scale = glm::vec3(scale.x, scale.y, scale.z);
        mesh->material = materials[scene->mMeshes[i]->mMaterialIndex];
        AddObject(mesh);
    }

    std::cout << "Scene Load Finished." << std::endl;
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
                mesh->material->SetUniform("_AmbientColor", ambientColor);

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