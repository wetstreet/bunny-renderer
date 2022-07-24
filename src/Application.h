#include "imgui/imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "ImGuizmo/ImGuizmo.h"

#include "common/Scene.h"
#include "common/Texture.h"
#include "common/Selection.h"
#include "common/DirectionalLight.h"
#include "common/Utils.h"
#include "common/Dialog.h"

#include "opengl/OpenGLRenderer.h"
#include "rasterizer/RasterizerRenderer.h"
#include "raytracer/RayTracerRenderer.h"

void SetDarkThemeColors()
{
	auto& colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

	// Headers
	colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	// Buttons
	colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	// Frame BG
	colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	// Tabs
	colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
	colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
	colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

	// Title
	colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
}

class Application
{
public:
	Application(Camera &camera, Scene &scene, OpenGLRenderer &openglRenderer, RasterizerRenderer &rasterizerRenderer, RayTracerRenderer& raytracer)
		: camera(camera), scene(scene), openglRenderer(openglRenderer), rasterizerRenderer(rasterizerRenderer), raytracer(raytracer)
	{
	}

	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%g");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%g");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%g");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	void Application::DrawEditor()
	{
		DrawMenu();

		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		DrawSettings();

		DrawInspector();

		DrawImage();

		DrawHierarchy();

		DrawScene();

		DrawToolBar();
	}

	void Application::DrawMenu()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Load Scene"))
				{
					std::string path = OpenFileDialog(3);
					if (path.size() != 0)
					{
						scene.LoadScene(path);
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Primitives"))
			{
				if (ImGui::MenuItem("Plane"))
				{
					node_clicked = scene.AddPrimitive("plane");
				}
				if (ImGui::MenuItem("Cube"))
				{
					node_clicked = scene.AddPrimitive("cube");
				}
				if (ImGui::MenuItem("Sphere"))
				{
					node_clicked = scene.AddPrimitive("sphere");
				}
				if (ImGui::MenuItem("Cylinder"))
				{
					node_clicked = scene.AddPrimitive("cylinder");
				}
				if (ImGui::MenuItem("Custom"))
				{
					std::string path = OpenFileDialog();
					if (path.size() != 0)
					{
						std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(path.c_str());
						node_clicked = scene.AddObject(mesh);
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Lights"))
			{
				if (ImGui::MenuItem("Directional Light"))
				{
					std::shared_ptr<DirectionalLight> dirLight = std::make_shared<DirectionalLight>();
					node_clicked = scene.AddObject(dirLight);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window"))
			{
				if (ImGui::MenuItem("Inspector")) showInspector = true;
				if (ImGui::MenuItem("Scene")) showScene = true;
				if (ImGui::MenuItem("Settings")) showSettings = true;
				if (ImGui::MenuItem("Render Result")) showImage = true;
				if (ImGui::MenuItem("Hierarchy")) showHierarchy = true;
				if (ImGui::MenuItem("Tool Bar")) showToolBar = true;
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	int gizmoType = (int)ImGuizmo::OPERATION::TRANSLATE;
	void Application::DrawInspector()
	{
		if (!ImGui::Begin("Inspector", &showInspector))
		{
			ImGui::End();
			return;
		}

		if (node_clicked != -1)
		{
			std::shared_ptr<Object> object = scene.objects[node_clicked];

			ImGui::Checkbox("##isEnabled", &object->isEnabled);
			ImGui::SameLine();
			ImGui::InputText("##name", object->name, IM_ARRAYSIZE(object->name));

			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				DrawVec3Control("Position", object->position);
				glm::vec3 rotation = glm::degrees(object->rotation);
				DrawVec3Control("Rotation", rotation);
				object->rotation = glm::radians(rotation);
				DrawVec3Control("Scale", object->scale, 1.0f);
				ImGui::Spacing();
			}

			if (object->GetType() == Type_Light)
			{
				if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
				{
					std::shared_ptr<Light> light = std::dynamic_pointer_cast<Light>(object);
					ImGui::ColorEdit3("Color", (float*)&light->color);
					ImGui::DragFloat("Intensity", &light->intensity, 0.1f);
				}
			}
			else if (object->GetType() == Type_Mesh)
			{
				std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(object);

				if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Text("Vertices: %i", mesh->vertices.size());
					ImGui::Text("Triangles: %i", mesh->indices.size());
					ImGui::Text("Path: %s", mesh->path.c_str());
				}
				if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
				{
					int item_current_idx = mesh->material->MaterialIndex;
					static char* items[] = { "Unlit", "Diffuse", "Normal" };
					ImGui::Combo("Shader", &item_current_idx, items, IM_ARRAYSIZE(items));
					if (item_current_idx != mesh->material->MaterialIndex)
					{
						// change material
						std::shared_ptr<Material> oldMat = mesh->material;
						switch (item_current_idx)
						{
						case 0:
							mesh->material = std::make_shared<UnlitMaterial>();
							break;
						case 1:
							mesh->material = std::make_shared<DiffuseMaterial>();
							break;
						case 2:
							mesh->material = std::make_shared<NormalMaterial>();
							break;
						}
						mesh->material->color = oldMat->color;
						mesh->material->texture = oldMat->texture;
						mesh->material->normalMap = oldMat->normalMap;
					}

					mesh->material->OnGUI();
				}
			}
		}

		ImGui::End();
	}

	void Application::DrawHierarchy()
	{
		if (!ImGui::Begin("Hierarchy", &showHierarchy))
		{
			ImGui::End();
			return;
		}

		static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

		static int selection_mask = 0;
		for (int i = 0; i < scene.objects.size(); i++)
		{
			std::shared_ptr<Object> object = scene.objects[i];
			ImGuiTreeNodeFlags node_flags = base_flags;
			const bool is_selected = (selection_mask & (1 << i)) != 0;
			if (is_selected)
				node_flags |= ImGuiTreeNodeFlags_Selected;

			node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet

			if (object->isEnabled)
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
			else
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 100));

			ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, object->name);
			if (ImGui::IsItemClicked())
				node_clicked = i;

			ImGui::PopStyleColor();
		}
		if (node_clicked != -1)
		{
			selection_mask = (1 << node_clicked);

			if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
			{
				scene.RemoveObject(node_clicked);
				node_clicked = -1;
			}
		}
		else {
			selection_mask = 0;
		}

		ImGui::End();
	}

	void Application::DrawGizmo()
	{
		if (node_clicked != -1 && node_clicked < scene.objects.size())
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			float windowWidth = (float)ImGui::GetWindowWidth();
			float windowHeight = (float)ImGui::GetWindowHeight();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

			std::shared_ptr<Object> object = scene.objects[node_clicked];

			ImGuizmo::Manipulate(glm::value_ptr(scene.camera.view), glm::value_ptr(scene.camera.projection),
				(ImGuizmo::OPERATION)gizmoType, ImGuizmo::LOCAL, glm::value_ptr(object->objectToWorld));

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				DecomposeTransform(object->objectToWorld, translation, rotation, scale);
				object->position = translation;
				object->rotation = rotation;
				object->scale = scale;
			}
		}
	}

	void Application::ClickToSelect()
	{
		// when sth is already selected and mouse is over gizmo, skip selection
		if (ImGuizmo::IsOver() && node_clicked != -1)
			return;

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			ImVec2 mousePos = ImGui::GetMousePos();
			mousePos -= windowPos;
			mousePos.y = viewport.y - mousePos.y;
			int mouseX = (int)mousePos.x;
			int mouseY = (int)mousePos.y;

			if (mouseX >= 0 && mouseY >= 0 && mouseX <= viewport.x && mouseY <= viewport.y)
			{
				node_clicked = openglRenderer.GetObjectID(mouseX, mouseY);
				if (node_clicked != -1 && (node_clicked < 0 || node_clicked > scene.objects.size()))
				{
					std::cout << "mouse picking failed!, id=" << node_clicked << ",mousex=" << mouseX << ",mousey=" << mouseY << std::endl;
					node_clicked = -1;
				}
			}
		}
	}

	void Application::DrawScene()
	{
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Scene", &showScene))
		{
			ImGui::End();
			return;
		}
		// todo: skip scene camera manipulation when scene is not visible

		ImGui::BeginChild("GameRender");
		viewport = ImGui::GetWindowSize();
		windowPos = ImGui::GetWindowPos();

		// scroll and click when hovered
		if (ImGui::IsWindowHovered())
		{
			if (ImGui::GetIO().MouseWheel != 0)
				scene.camera.Scroll(ImGui::GetIO().MouseWheel);

			ClickToSelect();
		}

		scene.camera.windowPos = glm::ivec2(windowPos.x, windowPos.y);
		scene.camera.viewport = glm::ivec2(viewport.x, viewport.y);
		openglRenderer.viewport = glm::vec2(viewport.x, viewport.y);

		GLuint rtID = postprocess ? openglRenderer.postprocessRT : openglRenderer.renderTexture;

		// Because I use the texture from OpenGL, I need to invert the V from the UV.
		ImGui::Image((ImTextureID)(intptr_t)rtID, viewport, ImVec2(0, 1), ImVec2(1, 0));

		DrawGizmo();

		ImGui::EndChild();

		ImGui::End();
	}

	void Application::DrawSettings()
	{
		if (!ImGui::Begin("Settings", &showSettings))
		{
			ImGui::End();
			return;
		}

		//ImGui::Checkbox("Post Process", &postprocess);

		ImGui::ColorEdit3("Ambient", (float*)&scene.ambientColor);


		if (ImGui::CollapsingHeader("Scene Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::InputFloat3("Position", (float*)&camera.Position);
			ImGui::InputFloat("Move Speed", (float*)&camera.speed);
			ImGui::InputFloat("Sensitivity", (float*)&camera.sensitivity);
			ImGui::InputFloat("Pan Speed", (float*)&camera.scenePanSpeed);
			ImGui::InputFloat("Scroll Speed", (float*)&camera.sceneScrollSpeed);
			ImGui::InputFloat("Near Plane", (float*)&camera.nearPlane);
			ImGui::InputFloat("Far Plane", (float*)&camera.farPlane);
			ImGui::ColorEdit3("Background", (float*)&camera.clearColor);
		}

		if (ImGui::CollapsingHeader("Raytracer", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::InputInt("Samples Per Pixel", &raytracer.samples_per_pixel);
			ImGui::InputInt("Max Depth", &raytracer.max_depth);
			ImGui::InputInt2("Viewport", (int*)&raytracer_viewport);
		}

		ImGui::End();
	}

	void Application::DrawToolBar()
	{
		ImGuiWindowClass window_class;
		window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
		ImGui::SetNextWindowClass(&window_class);
		if (!ImGui::Begin("Tool Bar", &showToolBar, ImGuiWindowFlags_NoDecoration))
		{
			ImGui::End();
			return;
		}

		ImGui::RadioButton("Move", &gizmoType, (int)ImGuizmo::OPERATION::TRANSLATE); ImGui::SameLine();
		ImGui::RadioButton("Rotate", &gizmoType, (int)ImGuizmo::OPERATION::ROTATE); ImGui::SameLine();
		ImGui::RadioButton("Scale", &gizmoType, (int)ImGuizmo::OPERATION::SCALE); ImGui::SameLine();

		if (ImGui::Button("Rasterize"))
		{
			rasterizerRenderer.viewport.x = (int)viewport.x;
			rasterizerRenderer.viewport.y = (int)viewport.y;

			content_size = ImVec2((float)rasterizerRenderer.viewport.x, (float)rasterizerRenderer.viewport.y);

			double startTime = glfwGetTime();

			rasterizerRenderer.Render(scene);

			double deltaTime = glfwGetTime() - startTime;
			std::cout << "Render finished, took " << deltaTime << " seconds." << std::endl;

			showImage = true;

			ImGui::SetWindowFocus("Render Result");
		}
		ImGui::SameLine();

		if (ImGui::Button("RayTrace"))
		{
			raytracer.viewport.x = raytracer_viewport.x;
			raytracer.viewport.y = raytracer_viewport.y;

			content_size = ImVec2((float)raytracer.viewport.x, (float)raytracer.viewport.y);

			raytracer.startTime = glfwGetTime();

			raytracer.RenderAsync(scene, [this] {
				std::cout << "Rendering Finished, took " << glfwGetTime() - raytracer.startTime << "s" << std::endl;
			});

			showRaytraceResult = true;
		}

		ImGui::SameLine();
		if (ImGui::Button("Reload Shader"))
		{
			Shader::normalShader->ReloadShader();
			Shader::skyboxShader->ReloadShader();
			std::cout << "Reloaded Shaders." << std::endl;
		}

		//ImGui::SameLine();
		//if (ImGui::Button("Irradiance"))
		//{
		//	openglRenderer.GenerateIrradianceMap(scene);
		//}

		ImGui::SameLine();
		ImGui::Checkbox("Show Irradiance", &scene.skybox.showIrradianceMap);

		ImGui::SameLine(ImGui::GetWindowWidth() - 200);
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::End();
	}

	void Application::DrawImage()
	{
		if (showImage)
		{
			ImGui::SetNextWindowSize(content_size + ImVec2(20, 35));
			ImGui::Begin("Render Result", &showImage);
			//ImGui::Image((ImTextureID)(intptr_t)rasterizerRenderer.depthTexture, content_size, ImVec2(0, 1), ImVec2(1, 0)); // visualize depth buffer
			ImGui::Image((ImTextureID)(intptr_t)rasterizerRenderer.renderTexture, content_size, ImVec2(0, 1), ImVec2(1, 0));
			ImGui::End();
		}
		if (showRaytraceResult)
		{
			ImGui::SetNextWindowSize(content_size + ImVec2(20, 35));
			ImGui::Begin("Raytracer", &showRaytraceResult);

			glBindTexture(GL_TEXTURE_2D, raytracer.renderTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, raytracer.viewport.x, raytracer.viewport.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, raytracer.pixels);

			ImGui::Image((ImTextureID)(intptr_t)raytracer.renderTexture, content_size, ImVec2(0, 1), ImVec2(1, 0));
			ImGui::End();
		}
	}

private:
	Camera &camera;
	Scene &scene;
	OpenGLRenderer &openglRenderer;
	RasterizerRenderer &rasterizerRenderer;
	RayTracerRenderer &raytracer;

	ImVec2 viewport = ImVec2(800, 800);
	ImVec2 windowPos;

	glm::ivec2 raytracer_viewport = glm::ivec2(1200, 600);

	ImVec2 window_size;
	ImVec2 content_size;

	// regular window
	bool showInspector = true;
	bool showScene = true;
	bool showSettings = true;
	bool showHierarchy = true;
	bool showRasterizer = true;
	bool showToolBar = true;

	bool postprocess = false;

	// special window
	bool showImage = false;
	bool showRaytraceResult = false;
};