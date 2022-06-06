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

class Application
{
public:
	Application(Camera &camera, Scene &scene, OpenGLRenderer &openglRenderer, RasterizerRenderer &rasterizerRenderer, RayTracerRenderer& raytracer)
		: camera(camera), scene(scene), openglRenderer(openglRenderer), rasterizerRenderer(rasterizerRenderer), raytracer(raytracer)
	{
		strcpy(customMeshName, "");
	}

	void Application::DrawEditor()
	{
		DrawMenu();

		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		DrawInspector();

		DrawImage();

		DrawSceneCamera();

		DrawHierarchy();

		DrawScene();

		DrawToolBar();
	}

	void Application::DrawMenu()
	{
		if (ImGui::BeginMainMenuBar())
		{
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
						mesh->SetName(GetFileNameFromPath(path).c_str());
						mesh->texture = scene.white_tex;
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
					dirLight->SetName("directional light");
					node_clicked = scene.AddObject(dirLight);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window"))
			{
				if (ImGui::MenuItem("Inspector")) showInspector = true;
				if (ImGui::MenuItem("Scene")) showScene = true;
				if (ImGui::MenuItem("Scene Camera")) showSceneCamera = true;
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
		if (showInspector)
		{
			ImGui::Begin("Inspector", &showInspector);

			if (node_clicked != -1)
			{
				std::shared_ptr<Object> object = scene.objects[node_clicked];

				ImGui::Checkbox("##isEnabled", &object->isEnabled);
				ImGui::SameLine();
				ImGui::InputText("##name", object->name, IM_ARRAYSIZE(object->name));

				if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
				{
					if (ImGui::Button("P"))
					{
						object->position.x = 0;
						object->position.y = 0;
						object->position.z = 0;
					}
					ImGui::SameLine();
					ImGui::DragFloat3("Position", (float*)&object->position, 0.01f);

					glm::vec3 degrees = glm::degrees(object->rotation);
					if (ImGui::Button("R"))
					{
						degrees.x = 0;
						degrees.y = 0;
						degrees.z = 0;
					}
					ImGui::SameLine();
					ImGui::DragFloat3("Rotation", (float*)&degrees, 0.01f);
					object->rotation = glm::radians(degrees);

					if (ImGui::Button("S"))
					{
						object->scale.x = 1;
						object->scale.y = 1;
						object->scale.z = 1;
					}
					ImGui::SameLine();
					ImGui::DragFloat3("Scale", (float*)&object->scale, 0.01f);
				}

				if (object->GetType() == Type_Light)
				{
					if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
					{
						std::shared_ptr<Light> light = std::dynamic_pointer_cast<Light>(object);
						ImGui::ColorEdit3("Color", (float*)&light->color);
						ImGui::InputFloat("Intensity", &light->intensity);
					}
				}
				else if (object->GetType() == Type_Mesh)
				{
					if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
					{
						std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(object);
						std::stringstream ss;
						ss << "Vertices: " << mesh->vertices.size() << std::endl
							<< "Triangles: " << (mesh->indices.size() / 3) << std::endl
							<< "Path: " << mesh->path;
						ImGui::Text(ss.str().c_str());
						ss.clear(); ss.str("");

						ImGui::Separator();

						Texture& tex = *mesh->texture;
						ss << GetFileNameFromPath(tex.path) << std::endl << "Size: " << tex.width << "x" << tex.height;
						ImGui::Text(ss.str().c_str());
						ss.clear(); ss.str("");
						if (ImGui::ImageButton((ImTextureID)(intptr_t)tex.ID, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0)))
						{
							std::string s = OpenFileDialog(1);
							if (s.size() > 0)
							{
								mesh->texture = std::make_shared<Texture>(s.c_str(), GL_TEXTURE0);
							}
						}

						ImGui::Separator();
						GLuint ID = 0;
						if (mesh->normalMap)
						{
							Texture& normalMap = *mesh->normalMap;
							ss << GetFileNameFromPath(normalMap.path) << std::endl << "Size: " << normalMap.width << "x" << normalMap.height;
							ImGui::Text(ss.str().c_str());
							ss.clear(); ss.str("");
							ID = normalMap.ID;
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
								mesh->normalMap = std::make_shared<Texture>(s.c_str(), GL_TEXTURE1);
							}
						}
						if (mesh->normalMap)
						{
							ImGui::SameLine();

							ImGui::PushID(0);
							ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0.6f, 0.6f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0.7f, 0.7f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0.8f, 0.8f));
							if (ImGui::Button("Clear"))
							{
								mesh->normalMap = nullptr;
							}
							ImGui::PopStyleColor(3);
							ImGui::PopID();
						}
					}
				}
			}

			ImGui::End();
		}
	}

	void Application::DrawHierarchy()
	{
		if (showHierarchy)
		{
			ImGui::Begin("Hierarchy", &showHierarchy);

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
				// Update selection state
				// (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
				if (ImGui::GetIO().KeyCtrl)
					selection_mask ^= (1 << node_clicked);          // CTRL+click to toggle
				else //if (!(selection_mask & (1 << node_clicked))) // Depending on selection behavior you want, may want to preserve selection when clicking on item that is part of the selection
					selection_mask = (1 << node_clicked);           // Click to single-select

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
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver())
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
		if (showScene)
		{
			ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
			bool visible = ImGui::Begin("Scene", &showScene);
			if (!visible)
			{
				ImGui::End();
				return;
			}
			// todo: skip scene camera manipulation when scene is not visible

			ImGui::BeginChild("GameRender");
			viewport = ImGui::GetWindowSize();
			windowPos = ImGui::GetWindowPos();

			scene.camera.windowPos = glm::vec2(windowPos.x, windowPos.y);
			scene.camera.viewport = glm::vec2(viewport.x, viewport.y);
			openglRenderer.viewport = glm::vec2(viewport.x, viewport.y);

			GLuint rtID = postprocess ? openglRenderer.postprocessRT : openglRenderer.renderTexture;

			// Because I use the texture from OpenGL, I need to invert the V from the UV.
			ImGui::Image((ImTextureID)(intptr_t)rtID, viewport, ImVec2(0, 1), ImVec2(1, 0));

			ClickToSelect();

			DrawGizmo();

			ImGui::EndChild();

			ImGui::End();
		}
	}

	void Application::DrawSceneCamera()
	{
		if (showSceneCamera)
		{
			ImGui::Begin("Scene Camera", &showSceneCamera);

			ImGui::InputFloat3("Position", (float*)&camera.Position);
			ImGui::InputFloat("Move Speed", (float*)&camera.speed);
			ImGui::InputFloat("Sensitivity", (float*)&camera.sensitivity);
			ImGui::InputFloat("Pan Speed", (float*)&camera.scenePanSpeed);
			ImGui::InputFloat("Scroll Speed", (float*)&camera.sceneScrollSpeed);

			ImGui::ColorEdit3("Background", (float*)&camera.clearColor);

			ImGui::Checkbox("Post Process", &postprocess);

			ImGui::End();
		}
	}

	void Application::DrawToolBar()
	{
		if (showToolBar)
		{
			ImGui::Begin("Tool Bar", &showToolBar, ImGuiWindowFlags_NoDecoration);

			ImGui::RadioButton("Move", &gizmoType, (int)ImGuizmo::OPERATION::TRANSLATE); ImGui::SameLine();
			ImGui::RadioButton("Rotate", &gizmoType, (int)ImGuizmo::OPERATION::ROTATE); ImGui::SameLine();
			ImGui::RadioButton("Scale", &gizmoType, (int)ImGuizmo::OPERATION::SCALE); ImGui::SameLine();

			if (ImGui::Button("Rasterize"))
			{
				rasterizerRenderer.viewport.x = viewport.x;
				rasterizerRenderer.viewport.y = viewport.y;

				window_size = ImVec2(rasterizerRenderer.viewport.x + 20, rasterizerRenderer.viewport.y + 35);
				content_size = ImVec2(rasterizerRenderer.viewport.x, rasterizerRenderer.viewport.y);

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

				raytracer.viewport.x = viewport.x;
				raytracer.viewport.y = viewport.y;

				window_size = ImVec2(raytracer.viewport.x + 20, raytracer.viewport.y + 35);
				content_size = ImVec2(raytracer.viewport.x, raytracer.viewport.y);

				raytracer.startTime = glfwGetTime();

				raytracer.RenderAsync(scene, [this] {
					std::cout << "Rendering Finished, took " << glfwGetTime() - raytracer.startTime << "s" << std::endl;
				});

				showRaytraceResult = true;
			}

			ImGui::SameLine(ImGui::GetWindowWidth() - 200);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			ImGui::End();
		}
	}

	void Application::DrawImage()
	{
		if (showImage)
		{
			ImGui::SetNextWindowSize(window_size);
			ImGui::Begin("Render Result", &showImage);
			//ImGui::Image((ImTextureID)(intptr_t)rasterizerRenderer.depthTexture, content_size, ImVec2(0, 1), ImVec2(1, 0)); // visualize depth buffer
			ImGui::Image((ImTextureID)(intptr_t)rasterizerRenderer.renderTexture, content_size, ImVec2(0, 1), ImVec2(1, 0));
			ImGui::End();
		}
		if (showRaytraceResult)
		{
			ImGui::SetNextWindowSize(window_size);
			ImGui::Begin("Raytracer", &showRaytraceResult);

			glBindTexture(GL_TEXTURE_2D, raytracer.renderTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport.x, viewport.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, raytracer.pixels);

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

	ImVec2 window_size;
	ImVec2 content_size;

	// regular window
	bool showInspector = true;
	bool showScene = true;
	bool showSceneCamera = true;
	bool showHierarchy = true;
	bool showRasterizer = true;
	bool showToolBar = true;

	bool postprocess = false;

	// special window
	bool showImage = false;
	bool showRaytraceResult = false;
	bool showCustomMeshPopup = false;
	char customMeshName[32];
};