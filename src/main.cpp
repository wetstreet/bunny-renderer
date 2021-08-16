#include <iostream>

#include "opengl/Scene.h"
#include "opengl/Texture.h"
#include "rasterizer/rasterizer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

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

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	camera = new Camera(width, height, glm::vec3(0.0f, 0.0f, 5.0f));

	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset)
	{
		camera->ScrollCallback(window, xoffset, yoffset);
	});
	
	Shader shaderProgram("src/opengl/shaders/default.vert", "src/opengl/shaders/default.frag");

	Scene scene(camera);

	Texture head_diffuse("obj/african_head/african_head_diffuse.tga", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
	Texture white_tex("obj/white_texture.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);

	Mesh head("obj/african_head/african_head.obj");
	head.texture = &head_diffuse;
	head.shader = &shaderProgram;
	head.name = "head";
	Mesh cube("obj/cube.obj");
	cube.texture = &white_tex;
	cube.shader = &shaderProgram;
	cube.name = "cube";
	// cube.position.x = 1.0f;
	// cube.rotation.y = 45;
	// cube.scale.y = 2;

	// scene.AddMesh(head);
	scene.AddMesh(cube);

	glEnable(GL_DEPTH_TEST);

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// generate texture
	GLuint texColorBuffer;
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 800, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0); 

	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 800);  
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	ImVec2 viewport(800, 800);
	ImVec2 windowPos;

	double lastTime = glfwGetTime();

	bool showImage = false;
	GLuint image_texture;
	
	Rasterizer ras;
	ImVec2 window_size;
	ImVec2 content_size(ras.size.x, ras.size.y);

	while (!glfwWindowShouldClose(window))
	{
    	double nowTime = glfwGetTime();
		double deltaTime = nowTime - lastTime;
		lastTime = nowTime;

		glBindTexture(GL_TEXTURE_2D, texColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewport.x, viewport.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, viewport.x, viewport.y);  
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glViewport(0, 0, viewport.x, viewport.y);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera->windowPos = glm::vec2(windowPos.x, windowPos.y);
		camera->viewport = glm::vec2(viewport.x, viewport.y);
		camera->SceneInputs(window, deltaTime);
		camera->updateMatrix(45.0f, 0.1f, 1000.0f, viewport.x, viewport.y);

		scene.Draw();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClearColor(0.22f, 0.22f, 0.22f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
		
        {
            ImGui::Begin("Inspector");

			if (node_clicked != -1)
			{
				Mesh *mesh = scene.meshes[node_clicked];
				ImGui::InputFloat3("Tr", (float*)&mesh->position);
				ImGui::InputFloat3("Rt", (float*)&mesh->rotation);
				ImGui::InputFloat3("Sc", (float*)&mesh->scale);
			}

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			ImGui::InputInt2("Size", (int*)&ras.size);
			if (ImGui::Button("rasterizer render"))
			{
				window_size = ImVec2(ras.size.x + 20, ras.size.y + 35);
				content_size = ImVec2(ras.size.x, ras.size.y);

				uint8_t* pixels = new uint8_t[ras.size.x * ras.size.y * 3];
				ras.scene = &scene;
				ras.width = ras.size.x;
				ras.height = ras.size.y;
				ras.Render(pixels);

				// Create a OpenGL texture identifier
				glGenTextures(1, &image_texture);
				glBindTexture(GL_TEXTURE_2D, image_texture);

				// Setup filtering parameters for display
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

				// Upload pixels into texture
			#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
				glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			#endif
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ras.size.x, ras.size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

				showImage = true;
				delete pixels;
			}
			
            ImGui::End();
        }

		if (showImage)
		{
			ImGui::SetNextWindowSize(window_size);
			ImGui::Begin("Render Result", &showImage);
			ImGui::Image((void*)(intptr_t)image_texture, content_size);
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
			for (int i = 0; i < scene.meshes.size(); i++)
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
				ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, scene.meshes[i]->name.c_str());
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

			// Because I use the texture from OpenGL, I need to invert the V from the UV.
			ImGui::Image((ImTextureID)(intptr_t)texColorBuffer, viewport, ImVec2(0, 1), ImVec2(1, 0));
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

	head_diffuse.Delete();
	white_tex.Delete();
	shaderProgram.Delete();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	delete camera;

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}