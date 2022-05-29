#include "Application.h"
#include "common/Dialog.h"

void Application::DrawEditor()
{
	DrawMenu();

	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

	DrawInspector();

	DrawRasterizer();

	DrawImage();

	DrawSceneCamera();

	DrawHierarchy();

	DrawScene();
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
			if (ImGui::MenuItem("SceneCamera")) showSceneCamera = true;
			if (ImGui::MenuItem("Rasterizer")) showRasterizer = true;
			if (ImGui::MenuItem("Hierarchy")) showHierarchy = true;
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

		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		if (node_clicked != -1)
		{
			std::shared_ptr<Object> object = scene.objects[node_clicked];
			
			ImGui::Checkbox("##isEnabled", &object->isEnabled);
			ImGui::SameLine();
			ImGui::InputText("##name", object->name, IM_ARRAYSIZE(object->name));

			ImGui::InputFloat3("Tr", (float*)&object->position);
			glm::vec3 degrees = glm::degrees(object->rotation);
			ImGui::InputFloat3("Rt", (float*)&degrees);
			object->rotation = glm::radians(degrees);
			ImGui::InputFloat3("Sc", (float*)&object->scale);


			ImGui::RadioButton("Translate", &gizmoType, (int)ImGuizmo::OPERATION::TRANSLATE); ImGui::SameLine();
			ImGui::RadioButton("Rotate", &gizmoType, (int)ImGuizmo::OPERATION::ROTATE); ImGui::SameLine();
			ImGui::RadioButton("Scale", &gizmoType, (int)ImGuizmo::OPERATION::SCALE);


			if (object->GetType() == Type_Light)
			{
				ImGui::Text("------------Light------------");
				std::shared_ptr<Light> light = std::dynamic_pointer_cast<Light>(object);
				ImGui::ColorEdit3("Color", (float*)&light->color);
				ImGui::InputFloat("Intensity", &light->intensity);
			}
			else if (object->GetType() == Type_Mesh)
			{
				ImGui::Text("------------Mesh------------");
				std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(object);
				ImGui::Text("Stats:");
				std::stringstream ss;
				ss << "Vertices: " << mesh->vertices.size() << std::endl
					<< "Triangles: " << (mesh->indices.size() / 3) << std::endl
					<< "Path: " << mesh->path;
				ImGui::Text(ss.str().c_str());
				ss.clear();
				ss.str("");

				ImGui::Spacing();
				ImGui::Text("Texture:");
				if (ImGui::ImageButton((ImTextureID)(intptr_t)mesh->texture->ID, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0)))
				{
					std::string s = OpenFileDialog(1);
					if (s.size() > 0)
					{
						std::shared_ptr<Texture> tex = std::make_shared<Texture>(s.c_str(), GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
						mesh->texture = tex;
					}
				}
				Texture& tex = *mesh->texture;
				ss << GetFileNameFromPath(tex.path) << std::endl << "Size: " << tex.width << "x" << tex.height ;
				ImGui::Text(ss.str().c_str());
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
		}
	}
}

void Application::DrawScene()
{
	if (showScene)
	{
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		ImGui::Begin("Scene", &showScene);

		// Using a Child allow to fill all the space of the window.
		// It also alows customization
		ImGui::BeginChild("GameRender");

		// Get the size of the child (i.e. the whole draw size of the windows).
		viewport = ImGui::GetWindowSize();
		windowPos = ImGui::GetWindowPos();

		scene.camera.windowPos = glm::vec2(windowPos.x, windowPos.y);
		scene.camera.viewport = glm::vec2(viewport.x, viewport.y);
		openglRenderer.viewport = glm::vec2(viewport.x, viewport.y);

		// Because I use the texture from OpenGL, I need to invert the V from the UV.
		if (postprocess)
			ImGui::Image((ImTextureID)(intptr_t)openglRenderer.outlineRT, viewport, ImVec2(0, 1), ImVec2(1, 0));
		else
			ImGui::Image((ImTextureID)(intptr_t)openglRenderer.renderTexture, viewport, ImVec2(0, 1), ImVec2(1, 0));

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

void Application::DrawImage()
{
	if (showImage)
	{
		ImGui::SetNextWindowSize(window_size);
		ImGui::Begin("Render Result", &showImage);
		ImGui::Image((ImTextureID)(intptr_t)rasterizerRenderer.renderTexture, content_size);
		ImGui::End();
	}
}

void Application::DrawRasterizer()
{
	if (showRasterizer)
	{
		ImGui::Begin("Rasterizer", &showRasterizer);

		ImGui::InputInt2("Size", (int*)&rasterizerRenderer.viewport);
		ImGui::SameLine();
		if (ImGui::Button("Sync"))
		{
			rasterizerRenderer.viewport.x = viewport.x;
			rasterizerRenderer.viewport.y = viewport.y;
		}

		if (ImGui::Button("rasterizer render"))
		{
			window_size = ImVec2(rasterizerRenderer.viewport.x + 20, rasterizerRenderer.viewport.y + 35);
			content_size = ImVec2(rasterizerRenderer.viewport.x, rasterizerRenderer.viewport.y);

			rasterizerRenderer.Render(scene);

			showImage = true;
		}

		ImGui::End();
	}
}