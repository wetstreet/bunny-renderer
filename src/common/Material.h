#ifndef __MATERIAL_H__
#define __MATERIAL_H__

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
#include "glm/gtc/type_ptr.hpp"
#include <sstream>

class Material
{
public:
    glm::vec4 color = glm::vec4(1, 1, 1, 1);
	std::shared_ptr<Texture> texture;
    std::shared_ptr<Texture> normalMap;

    Material() {}
    virtual void OnGUI() = 0;
	/*virtual Varying vertex(Vertex i);
	virtual glm::vec4 fragment(Varying& varying);*/
	virtual void Setup() = 0;

    void SetUniform(const GLchar* name, int i)
    {
        glUniform1i(glGetUniformLocation(shader->ID, name), i);
    }
    void SetUniform(const GLchar* name, glm::vec3& v)
    {
        glUniform3fv(glGetUniformLocation(shader->ID, name), 1, (float*)&v);
    }
    void SetUniform(const GLchar* name, glm::mat4& m)
    {
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, name), 1, GL_FALSE, glm::value_ptr(m));
    }
protected:
    std::stringstream ss;
	std::shared_ptr<Shader> shader;

    template<typename CreateFunc, typename ClearFunc>
    void DrawTextureUI(std::shared_ptr<Texture> texture, CreateFunc createFunc, ClearFunc clearFunc)
    {
        GLuint ID = 0;
        if (texture)
        {
            ss << GetFileNameFromPath(texture->path) << std::endl << "Size: " << texture->width << "x" << texture->height;
            ImGui::Text(ss.str().c_str());
            ss.clear(); ss.str("");
            ID = texture->ID;
        }
        else
        {
            ImGui::Text("no texture");
        }
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

            ImGui::PushID(0);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0.8f, 0.8f));
            if (ImGui::Button("Clear"))
            {
                clearFunc();
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }
    }
};

class UnlitMaterial : public Material
{
public:
    UnlitMaterial()
    {
        shader = Shader::unlitShader;
    }

    virtual void Setup()
    {
        shader->Activate();

        std::shared_ptr<Texture> tex = texture != nullptr ? texture : Texture::white_tex;
        tex->texUnit(*shader, "tex0", 0);
        tex->Bind();

        glUniform4fv(glGetUniformLocation(shader->ID, "_Color"), 1, (float*)&color);
    }

    virtual void OnGUI()
    {
        ImGui::ColorEdit4("Color", (float*)&color);

        DrawTextureUI(texture, [this](const char* path) {texture = std::make_shared<Texture>(path, GL_TEXTURE0); }, [this]() {texture = nullptr; });
    }
};

class DiffuseMaterial : public Material
{
public:
	DiffuseMaterial()
	{
		shader = Shader::defaultShader;
	}

	virtual void Setup()
	{
        shader->Activate();

        std::shared_ptr<Texture> tex = texture != nullptr ? texture : Texture::white_tex;
        tex->texUnit(*shader, "tex0", 0);
        tex->Bind();

        glUniform4fv(glGetUniformLocation(shader->ID, "_Color"), 1, (float*)&color);
	}

    virtual void OnGUI()
    {
        ImGui::ColorEdit4("Color", (float*)&color);

        DrawTextureUI(texture, [this](const char* path) {texture = std::make_shared<Texture>(path, GL_TEXTURE0); }, [this]() {texture = nullptr; });
    }
};

class NormalMaterial : public Material
{
public:
    NormalMaterial()
    {
        shader = Shader::normalShader;
    }

    virtual void Setup()
    {
        shader->Activate();

        std::shared_ptr<Texture> tex = texture != nullptr ? texture : Texture::white_tex;
        tex->texUnit(*shader, "tex0", 0);
        tex->Bind();

        std::shared_ptr<Texture> normal = normalMap != nullptr ? normalMap : Texture::white_tex;
        normal->texUnit(*shader, "normalMap", 0);
        normal->Bind();

        glUniform4fv(glGetUniformLocation(shader->ID, "_Color"), 1, (float*)&color);
    }

    virtual void OnGUI()
    {
        ImGui::ColorEdit4("Color", (float*)&color);

        DrawTextureUI(texture, [this](const char* path) {texture = std::make_shared<Texture>(path, GL_TEXTURE0); }, [this]() {texture = nullptr; });
        DrawTextureUI(normalMap, [this](const char* path) {normalMap = std::make_shared<Texture>(path, GL_TEXTURE1); }, [this]() {normalMap = nullptr; });
    }
};

#endif //__MATERIAL_H__