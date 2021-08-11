#include <iostream>

#include "Mesh.h"
#include "Texture.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

const unsigned int width = 800;
const unsigned int height = 800;

Camera *camera;

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (camera != NULL)
		camera->Position += camera->Orientation * (float)yoffset;
}

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

	glfwSetScrollCallback(window, ScrollCallback);

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

    bool show_demo_window = false;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	
	Shader shaderProgram("src/opengl/shaders/default.vert", "src/opengl/shaders/default.frag");

	Mesh mesh("obj/african_head/african_head.obj");
	Mesh cube("obj/cube.obj");
	cube.position.x = 1.0f;
	cube.rotation.y = 45;
	cube.scale.y = 2;

	Texture popCat("obj/african_head/african_head_diffuse.tga", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
	Texture white_tex("obj/white_texture.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);

	glEnable(GL_DEPTH_TEST);

	camera = new Camera(width, height, glm::vec3(0.0f, 1.0f, 2.0f));

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

		camera->Inputs(window, deltaTime, glm::vec2(viewport.x, viewport.y), glm::vec2(windowPos.x, windowPos.y));
		camera->updateMatrix(45.0f, 0.1f, 1000.0f, viewport.x, viewport.y);

		mesh.Draw(shaderProgram, *camera, popCat);
		cube.Draw(shaderProgram, *camera, white_tex);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClearColor(0.22f, 0.22f, 0.22f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
		
        {
            ImGui::Begin("Inspector");
			
			ImGui::InputFloat3("Tr", (float*)&cube.position);
			ImGui::InputFloat3("Rt", (float*)&cube.rotation);
			ImGui::InputFloat3("Sc", (float*)&cube.scale);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

    	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		{
			ImGui::Begin("GameWindow");

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

	popCat.Delete();
	shaderProgram.Delete();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	delete camera;

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}