#include <iostream>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "opengl/Scene.h"
#include "opengl/Texture.h"

#include "Renderer.h"

const unsigned int width = 1200;
const unsigned int height = 800;

Camera *camera;
		
int node_clicked = -1;

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "BunnyRenderer", NULL, NULL);
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

	bool showImage = false;
	
	ImVec2 window_size;
	ImVec2 content_size(renderer.ras.size.x, renderer.ras.size.y);

	while (!glfwWindowShouldClose(window))
	{
    	double nowTime = glfwGetTime();
		double deltaTime = nowTime - lastTime;
		lastTime = nowTime;

		renderer.Render_OpenGL(window, deltaTime);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
		
        {
            ImGui::Begin("Inspector");

			if (node_clicked != -1)
			{
				Mesh *mesh = renderer.scene->meshes[node_clicked];
				ImGui::InputFloat3("Tr", (float*)&mesh->position);
				ImGui::InputFloat3("Rt", (float*)&mesh->rotation);
				ImGui::InputFloat3("Sc", (float*)&mesh->scale);
			}

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			ImGui::InputInt2("Size", (int*)&renderer.ras.size);
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

		{
			ImGui::Begin("Scene Camera");

			ImGui::InputFloat3("Position", (float*)&camera->Position);
			ImGui::InputFloat("Move Speed", (float*)&camera->speed);
			ImGui::InputFloat("Sensitivity", (float*)&camera->sensitivity);
			ImGui::InputFloat("Pan Speed", (float*)&camera->scenePanSpeed);
			ImGui::InputFloat("Scroll Speed", (float*)&camera->sceneScrollSpeed);
			
			ImGui::End();
		}

		{
			ImGui::Begin("Hierarchy");

            static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

			static int selection_mask = (1 << 2);
			for (int i = 0; i < renderer.scene->meshes.size(); i++)
			{
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
				ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, renderer.scene->meshes[i]->name.c_str());
				if (ImGui::IsItemClicked())
					node_clicked = i;
			}
			if (node_clicked != -1)
			{
				// Update selection state
				// (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
				if (ImGui::GetIO().KeyCtrl)
					selection_mask ^= (1 << node_clicked);          // CTRL+click to toggle
				else //if (!(selection_mask & (1 << node_clicked))) // Depending on selection behavior you want, may want to preserve selection when clicking on item that is part of the selection
					selection_mask = (1 << node_clicked);           // Click to single-select
			}

			ImGui::End();
		}

    	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		{
			ImGui::Begin("Scene");

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

	renderer.Delete();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	delete camera;

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}