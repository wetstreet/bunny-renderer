#include <iostream>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "opengl/Scene.h"
#include "opengl/Texture.h"

#include "common/Renderer.h"
#include "common/DirectionalLight.h"

const unsigned int width = 1200;
const unsigned int height = 800;

Camera *camera;
		
int node_clicked = -1;

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "Bunny Renderer", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	gladLoadGL();

	ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

	camera = new Camera(width, height, glm::vec3(0.0f, 0.0f, 5.0f));

	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset)
	{
		camera->ScrollCallback(window, xoffset, yoffset);
	});

	Renderer renderer = Renderer::GetInstance();

	renderer.Init_Scene(camera);
	renderer.Init_OpenGL();

	ImVec2 viewport(800, 800);
	ImVec2 windowPos;

	double lastTime = glfwGetTime();
	
	ImVec2 window_size;
	ImVec2 content_size(renderer.ras.size.x, renderer.ras.size.y);

	bool showInspector = true;
	bool showScene = true;
	bool showSceneCamera = true;
	bool showHierarchy = true;
	bool showRasterizer = true;

	bool showImage = false;

	while (!glfwWindowShouldClose(window))
	{
    	double nowTime = glfwGetTime();
		double deltaTime = nowTime - lastTime;
		lastTime = nowTime;

		renderer.Render_OpenGL(window, deltaTime);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Primitives"))
			{
            	if (ImGui::MenuItem("Plane"))
				{
					node_clicked = renderer.AddPrimitive("plane");
				}
            	if (ImGui::MenuItem("Cube"))
				{
					node_clicked = renderer.AddPrimitive("cube");
				}
            	if (ImGui::MenuItem("Sphere"))
				{
					node_clicked = renderer.AddPrimitive("sphere");
				}
            	if (ImGui::MenuItem("Cylinder"))
				{
					node_clicked = renderer.AddPrimitive("cylinder");
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Lights"))
			{
            	if (ImGui::MenuItem("Directional Light"))
				{
					DirectionalLight *dir = new DirectionalLight();
    				strcpy(dir->name, "directional light");
					node_clicked = renderer.scene->AddObject(dir);
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
				Object *object = renderer.scene->objects[node_clicked];
				
				ImGui::Checkbox("##isEnabled", &object->isEnabled);
				ImGui::SameLine();
				ImGui::InputText("##name", object->name, IM_ARRAYSIZE(object->name));

				ImGui::InputFloat3("Tr", (float*)&object->position);
				ImGui::InputFloat3("Rt", (float*)&object->rotation);
				ImGui::InputFloat3("Sc", (float*)&object->scale);

				if (object->GetType() == Type_Light)
				{
					Light *light = (Light*)object;

					ImGui::Text("------------Light------------");
					
					ImGui::ColorEdit3("Color", (float*)&light->color);
					ImGui::InputFloat("Intensity", &light->intensity);
				}
			}

            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			
            ImGui::End();
        }

		if (showRasterizer)
		{
            ImGui::Begin("Rasterizer", &showRasterizer);

			ImGui::InputInt2("Size", (int*)&renderer.ras.size);
			ImGui::SameLine();
			if (ImGui::Button("Sync"))
			{
				renderer.ras.size.x = viewport.x;
				renderer.ras.size.y = viewport.y;
			}

			if (ImGui::Button("rasterizer render"))
			{
				window_size = ImVec2(renderer.ras.size.x + 20, renderer.ras.size.y + 35);
				content_size = ImVec2(renderer.ras.size.x, renderer.ras.size.y);

				renderer.Render_Rasterizer();

    			showImage = true;
			}

            ImGui::End();
		}

		if (showImage)
		{
			ImGui::SetNextWindowSize(window_size);
			ImGui::Begin("Render Result", &showImage);
			ImGui::Image((void*)(intptr_t)renderer.image_texture, content_size);
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
			
			ImGui::ColorEdit3("Background", (float*)&renderer.clear_color);
			
			ImGui::End();
		}

		if (showHierarchy)
		{
			ImGui::Begin("Hierarchy", &showHierarchy);

            static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

			static int selection_mask = (1 << 2);
			for (int i = 0; i < renderer.scene->objects.size(); i++)
			{
				Object *object = renderer.scene->objects[i];
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
					renderer.scene->RemoveObject(node_clicked);
					node_clicked = -1;
				}
			}

			ImGui::End();
		}

		if (showScene)
		{
    		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
			ImGui::Begin("Scene", & showScene);

			// Using a Child allow to fill all the space of the window.
			// It also alows customization
			ImGui::BeginChild("GameRender");
			
			// Get the size of the child (i.e. the whole draw size of the windows).
			viewport = ImGui::GetWindowSize();
			windowPos = ImGui::GetWindowPos();

			renderer.scene->camera->windowPos = glm::vec2(windowPos.x, windowPos.y);
			renderer.scene->camera->viewport = glm::vec2(viewport.x, viewport.y);
			renderer.viewport = glm::vec2(viewport.x, viewport.y);


			// Because I use the texture from OpenGL, I need to invert the V from the UV.
			ImGui::Image((ImTextureID)(intptr_t)renderer.texColorBuffer, viewport, ImVec2(0, 1), ImVec2(1, 0));
			ImGui::EndChild();
			ImGui::End();
		}

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	delete camera;

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}