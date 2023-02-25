#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <sstream>
#include <vector>

#include "glm/gtc/type_ptr.hpp"

#include "imgui/imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Utils.h"
#include "Dialog.h"

#include "Shader.h"
#include "Texture.h"
#include "Vertex.h"
#include "Skybox.h"

typedef void(*TexBindFunc)(Texture&, unsigned int);
extern TexBindFunc TextureBindFunction;

class Material
{
public:
    char name[32];

    glm::vec4 color = glm::vec4(1, 1, 1, 1);
    float metallic = 0.0f;
    float roughness = 0.5f;
    float cutoff = 0.5f;

    bool doubleSided = false;

	std::shared_ptr<Texture> texture;
    std::shared_ptr<Texture> metallicMap;
    std::shared_ptr<Texture> normalMap;

    const int MaterialIndex = -1;

    // rasterize rendering properties (set by rasterizer)
    glm::mat4 MVP;
    glm::mat4 objectToWorld;
    glm::mat4 worldToObject;
    glm::vec3 lightDir;
    glm::vec3 lightColor;
    glm::vec3 ambientColor;

    Material(int index) : MaterialIndex(index){}

    virtual void Setup() = 0;
    virtual void vertex(Vertex i, float* o) = 0;
    virtual glm::vec4 fragment(float* o) = 0;
    virtual void OnGUI() = 0;

    void SetUniform(const GLchar* name, int i)
    {
        glUniform1i(glGetUniformLocation(shader->ID, name), i);
    }
    void SetUniform(const GLchar* name, float f)
    {
        glUniform1f(glGetUniformLocation(shader->ID, name), f);
    }
    void SetUniform(const GLchar* name, glm::vec3& v)
    {
        glUniform3fv(glGetUniformLocation(shader->ID, name), 1, (float*)&v);
    }
    void SetUniform(const GLchar* name, glm::vec4& v)
    {
        glUniform4fv(glGetUniformLocation(shader->ID, name), 1, (float*)&v);
    }
    void SetUniform(const GLchar* name, glm::mat4& m)
    {
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, name), 1, GL_FALSE, glm::value_ptr(m));
    }
protected:
	std::shared_ptr<Shader> shader;

    template<typename CreateFunc, typename ClearFunc>
    void DrawTextureUI(std::shared_ptr<Texture> texture, CreateFunc createFunc, ClearFunc clearFunc, int index = 0)
    {
        GLuint ID = 0;
        if (texture)
        {
            ImGui::Text(GetFileNameFromPath(texture->path).c_str());
            ImGui::Text("Size: %ix%i", texture->width, texture->height);
            ID = texture->ID;
        }
        else
        {
            ImGui::Text("no texture");
            ImGui::Text("Size: 0x0");
        }
        ImGui::PushID(index);
        if (ImGui::ImageButton((ImTextureID)(intptr_t)ID, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0)))
        {
            std::string s = OpenFileDialog(1);
            if (s.size() > 0)
            {
                createFunc(s.c_str());
            }
        }
        if (texture)
        {
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0.8f, 0.8f));
            if (ImGui::Button("Clear"))
            {
                clearFunc();
            }
            ImGui::PopStyleColor(3);
        }
        ImGui::PopID();
    }
};

class UnlitMaterial : public Material
{
public:
    UnlitMaterial() : Material(0)
    {
        shader = Shader::unlitShader;
    }

    virtual void Setup()
    {
        shader->Activate();

        std::shared_ptr<Texture> tex = texture != nullptr ? texture : Texture::white_tex;
        tex->texUnit(*shader, "tex0", 0);
        TextureBindFunction(*tex, 0);

        SetUniform("_Color", color);
    }

    virtual void vertex(Vertex i, float* o)
    {
        Varying* varying = (Varying*)o;
        varying->uv = i.texcoord;
        varying->position = MVP * glm::vec4(i.position, 1);
    }

    virtual glm::vec4 fragment(float* o)
    {
        Varying* varying = (Varying*)o;
        std::shared_ptr<Texture> tex = texture != nullptr ? texture : Texture::white_tex;
        glm::vec4 albedo = tex->tex2D(varying->uv);
        return albedo * color;
    }

    virtual void OnGUI()
    {
        ImGui::ColorEdit4("Color", (float*)&color);

        DrawTextureUI(texture, [this](const char* path) {texture = std::make_shared<Texture>(path); }, [this]() {texture = nullptr; });
    }
};

class PBRMaterial : public Material
{
public:
    PBRMaterial() : Material(1)
    {
        shader = Shader::pbrShader;
    }

    virtual void Setup()
    {
        shader->Activate();

        std::shared_ptr<Texture> tex = texture != nullptr ? texture : Texture::white_tex;
        tex->texUnit(*shader, "albedoMap", 0);
        TextureBindFunction(*tex, 0);

        std::shared_ptr<Texture> normal = normalMap != nullptr ? normalMap : Texture::normal_tex;
        normal->texUnit(*shader, "normalMap", 1);
        TextureBindFunction(*normal, 1);

        std::shared_ptr<Texture> metal = metallicMap != nullptr ? metallicMap : Texture::white_tex;
        metal->texUnit(*shader, "metalMap", 2);
        TextureBindFunction(*metal, 2);

        SetUniform("_Color", color);
        SetUniform("_Metallic", metallic);
        SetUniform("_Roughness", roughness);
        SetUniform("_Cutoff", cutoff);
    }

    virtual void vertex(Vertex i, float* o)
    {
        using namespace glm;
        using vec3 = glm::vec3;

        Varying* varying = (Varying*)o;
        varying->uv = i.texcoord;
        varying->position = MVP * vec4(i.position, 1);

        vec3 worldNormal = normalize(i.normal * (mat3)worldToObject);
        vec3 worldTangent = normalize((mat3)objectToWorld * i.tangent);
        vec3 worldBitangent = cross(i.normal, i.tangent);
        varying->TBN = mat3(worldTangent, worldBitangent, worldNormal);
    }

    virtual glm::vec4 fragment(float* o)
    {
        Varying* varying = (Varying*)o;
        std::shared_ptr<Texture> colorTex = texture != nullptr ? texture : Texture::white_tex;
        glm::vec4 albedo = colorTex->tex2D(varying->uv);

        std::shared_ptr<Texture> normalTex = normalMap != nullptr ? normalMap : Texture::normal_tex;
        glm::vec3 normal = normalTex->tex2D(varying->uv);
        normal = normal * 2.0f - 1.0f;
        normal = normalize(varying->TBN * normal);

        float nl = std::max(0.0f, glm::dot(normal, lightDir));

        return albedo * color * nl;
    }

    virtual void OnGUI()
    {
        ImGui::ColorEdit4("Color", (float*)&color);

        ImGui::SliderFloat("Metallic", &metallic, 0, 1);
        ImGui::SliderFloat("Roughness", &roughness, 0, 1);

        ImGui::Checkbox("Double Sided", &doubleSided);

        DrawTextureUI(texture, [this](const char* path) { texture = std::make_shared<Texture>(path); }, [this]() { texture = nullptr; });

        ImGui::Separator();

        DrawTextureUI(metallicMap, [this](const char* path) { metallicMap = std::make_shared<Texture>(path); }, [this]() { metallicMap = nullptr; }, 1);

        ImGui::Separator();

        DrawTextureUI(normalMap, [this](const char* path) { normalMap = std::make_shared<Texture>(path); }, [this]() { normalMap = nullptr; }, 2);
    }
};

class SkyboxMaterial : public Material
{
public:
    Skybox* skybox;

    SkyboxMaterial() : Material(3)
    {
        shader = Shader::skyboxShader;
    }

    virtual void Setup()
    {
    }

    virtual void vertex(Vertex i, float* o)
    {
        using namespace glm;
        using vec3 = glm::vec3;

        SkyboxVarying* varying = (SkyboxVarying*)o;
        vec4 pos = MVP * vec4(i.position, 1);
        varying->position = vec4(pos.x, pos.y, pos.w, pos.w);
        varying->uv = vec3(i.position.x, i.position.y, -i.position.z);
    }

    virtual glm::vec4 fragment(float* o)
    {
        SkyboxVarying* varying = (SkyboxVarying*)o;
        glm::vec3 uv = glm::normalize(varying->uv);
        return skybox->texCube_f(uv);
    }

    virtual void OnGUI()
    {

    }
};

#endif //__MATERIAL_H__