#include "Application.h"

void Application::DrawEditor(Camera* camera, Scene& scene, ImGuiIO& io, OpenGLRenderer& openglRenderer, RasterizerRenderer& rasterizerRenderer)
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
				showCustomMeshPopup = true;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Lights"))
		{
			if (ImGui::MenuItem("Directional Light"))
			{
				DirectionalLight* dir = new DirectionalLight();
				strcpy(dir->name, "directional light");
				node_clicked = scene.AddObject(dir);
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

	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

	if (showInspector)
	{
		ImGui::Begin("Inspector", &showInspector);

		if (node_clicked != -1)
		{
			Object* object = scene.objects[node_clicked];

			ImGui::Checkbox("##isEnabled", &object->isEnabled);
			ImGui::SameLine();
			ImGui::InputText("##name", object->name, IM_ARRAYSIZE(object->name));

			ImGui::InputFloat3("Tr", (float*)&object->position);
			ImGui::InputFloat3("Rt", (float*)&object->rotation);
			ImGui::InputFloat3("Sc", (float*)&object->scale);

			if (object->GetType() == Type_Light)
			{
				Light* light = (Light*)object;

				ImGui::Text("------------Light------------");

				ImGui::ColorEdit3("Color", (float*)&light->color);
				ImGui::InputFloat("Intensity", &light->intensity);
			}
		}

		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::End();
	}

	if (showCustomMeshPopup)
	{
		ImGui::Begin("Add Custom Mesh", &showCustomMeshPopup);

		ImGui::InputText("##name", customMeshName, IM_ARRAYSIZE(customMeshName));

		ImGui::SameLine();

		if (ImGui::Button("Ok"))
		{
			node_clicked = scene.AddPrimitive(customMeshName);
			strcpy(customMeshName, "");
			showCustomMeshPopup = false;
		}

		ImGui::End();
	}

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

	if (showImage)
	{
		ImGui::SetNextWindowSize(window_size);
		ImGui::Begin("Render Result", &showImage);
		ImGui::Image((ImTextureID)(intptr_t)rasterizerRenderer.renderTexture, content_size);
		ImGui::End();
	}

	if (showSceneCamera)
	{
		ImGui::Begin("Scene Camera", &showSceneCamera);

		ImGui::InputFloat3("Position", (float*)&camera->Position);
		ImGui::InputFloat("Move Speed", (float*)&camera->speed);
		ImGui::InputFloat("Sensitivity", (float*)&camera->sensitivity);
		ImGui::InputFloat("Pan Speed", (float*)&camera->scenePanSpeed);
		ImGui::InputFloat("Scroll Speed", (float*)&camera->sceneScrollSpeed);

		ImGui::ColorEdit3("Background", (float*)&camera->clearColor);

		ImGui::Checkbox("Post Process", &postprocess);

		ImGui::End();
	}

	if (showHierarchy)
	{
		ImGui::Begin("Hierarchy", &showHierarchy);

		static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

		static int selection_mask = (1 << 2);
		for (int i = 0; i < scene.objects.size(); i++)
		{
			Object* object = scene.objects[i];
			ImGuiTreeNodeFlags node_flags = base_flags;
			const bool is_selected = (selection_mask & (1 << i)) != 0;
			if (is_selected)
				node_flags |= ImGuiTreeNodeFlags_Selected;
			// bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "Selectable Node %d", i);
			// if (ImGui::IsItemClicked())
			// 	node_clicked = i;
			// if (node_open)
			// {
			// 	ImGui::TreePop();
			// }

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

		ImGui::End();
	}

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

		scene.camera->windowPos = glm::vec2(windowPos.x, windowPos.y);
		scene.camera->viewport = glm::vec2(viewport.x, viewport.y);
		openglRenderer.viewport = glm::vec2(viewport.x, viewport.y);

		// Because I use the texture from OpenGL, I need to invert the V from the UV.
		if (postprocess)
			ImGui::Image((ImTextureID)(intptr_t)openglRenderer.postprocessRT, viewport, ImVec2(0, 1), ImVec2(1, 0));
		else
			ImGui::Image((ImTextureID)(intptr_t)openglRenderer.renderTexture, viewport, ImVec2(0, 1), ImVec2(1, 0));

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			ImVec2 mouse = ImGui::GetIO().MousePos;

			if (mouse.x >= windowPos.x && mouse.x <= windowPos.x + viewport.x && mouse.y >= windowPos.y && mouse.y <= windowPos.y + viewport.y)
			{
				mouse -= windowPos;
				mouse.y = viewport.y - mouse.y;

				glm::vec3 ray_origin;
				glm::vec3 ray_direction;
				ScreenPosToWorldRay(
					mouse.x, mouse.y,
					viewport.x, viewport.y,
					camera->view,
					camera->projection,
					ray_origin,
					ray_direction
				);

				node_clicked = -1;
				for (int i = 0; i < scene.objects.size(); i++)
				{
					Object* object = scene.objects[i];
					if (object->GetType() == Type_Mesh)
					{
						Mesh* mesh = (Mesh*)object;

						float intersection_distance; // Output of TestRayOBBIntersection()

						if (TestRayOBBIntersection(
							ray_origin,
							ray_direction,
							mesh->minPos,
							mesh->maxPos,
							mesh->objectToWorld,
							intersection_distance)
							) {
							node_clicked = i;
							break;
						}
					}
				}
			}
		}

		ImGui::EndChild();

		if (node_clicked != -1)
		{
			// draw gizmo
			ImGui::BeginChild("GameRender");

			Object* object = scene.objects[node_clicked];

			ImDrawList* drawList = ImGui::GetWindowDrawList();

			glm::mat4 mat;

			bool local = false;
			if (local)
				mat = camera->cameraMatrix * object->objectToWorld; // local
			else
				mat = camera->cameraMatrix * glm::translate(object->position);

			ImVec2 origin = WorldToScreenPos(mat, glm::vec4(0, 0, 0, 1), windowPos, viewport);
			glm::vec4 mouse = makeVect(ImGui::GetIO().MousePos);

			int curAxis = -1;

			static const glm::vec4 	directionUnary[3] = { glm::vec4(1, 0, 0, 1), glm::vec4(0, 1, 0, 1), glm::vec4(0, 0, 1, 1) };
			static const ImU32 		directionColor[3] = { IM_COL32(255, 0, 0, 255), IM_COL32(0, 255, 0, 255), IM_COL32(0, 0, 255, 255) };
			static const ImU32 selectionColor = IM_COL32(255, 128, 16, 138);

			for (int i = 0; i < 3; i++)
			{
				ImVec2 axis = WorldToScreenPos(mat, directionUnary[i], windowPos, viewport);

				if (curAxis == -1)
				{
					glm::vec4 closestPointOnAxis = PointOnSegment(mouse, makeVect(origin), makeVect(axis));
					if (glm::length(closestPointOnAxis - mouse) < 5.f)
					{
						curAxis = i;
					}
				}

				ImU32 color = curAxis == i ? selectionColor : directionColor[i];
				drawList->AddLine(origin, axis, color, 3.f);
				DrawArrow(origin, axis, drawList, color);
			}

			if (mUsing)
			{
				std::cout << "click, axis=" << curAxis << std::endl;

				if (!io.MouseDown[0])
				{
					mUsing = false;
				}
			}
			else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && curAxis != -1)
			{
				mUsing = true;
			}

			drawList->AddCircleFilled(origin, 6.f, IM_COL32(255, 255, 255, 255), 32);

			ImGui::EndChild();
		}


		ImGui::End();
	}
}


void Application::DrawArrow(ImVec2 origin, ImVec2 worldDirSSpace, ImDrawList* drawList, ImU32 color)
{
	ImVec2 dir(origin - worldDirSSpace);

	float d = sqrtf(ImLengthSqr(dir));
	dir /= d; // Normalize
	dir *= 6.0f;

	ImVec2 ortogonalDir(dir.y, -dir.x); // Perpendicular vector
	ImVec2 a(worldDirSSpace + dir);
	drawList->AddTriangleFilled(worldDirSSpace - dir, a + ortogonalDir, a - ortogonalDir, color);
}