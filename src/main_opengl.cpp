#include "Application.h"
#include "opengl/OpenGLRenderer.h"

const unsigned int width = 1400;
const unsigned int height = 900;

std::unique_ptr<Camera> camera;

std::shared_ptr<OpenGLRenderer> openglRenderer;

void TextureRegister(Texture* tex) { openglRenderer->RegisterTexture(tex); }
void SkyboxRegister(Skybox* skybox) { openglRenderer->RegisterSkybox(skybox); }
void MeshRegister(Mesh* mesh) { openglRenderer->RegisterMesh(mesh); }

void TextureUnregister(Texture* tex) { openglRenderer->UnregisterTexture(tex); }
void MeshUnregister(Mesh* mesh) { openglRenderer->UnregisterMesh(mesh); }
void SkyboxUnregister(Skybox* skybox) { openglRenderer->UnregisterSkybox(skybox); }
void TextureBind(Texture& tex, GLuint slot) { openglRenderer->BindTexture(tex, slot); }

int main(int argc, char* argv[]) {
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "Bunny Renderer - OpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	gladLoadGL();

	TextureRegisterFunction = TextureRegister;
	SkyboxRegisterFunction = SkyboxRegister;
	MeshRegisterFunction = MeshRegister;

	TextureUnregisterFunction = TextureUnregister;
	SkyboxUnregisterFunction = SkyboxUnregister;
	MeshUnregisterFunction = MeshUnregister;
	TextureBindFunction = TextureBind;

	openglRenderer = std::make_shared<OpenGLRenderer>();

	ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	float fontSize = 18.0f;// *2.0f;
	io.Fonts->AddFontFromFileTTF("res/fonts/opensans/OpenSans-Bold.ttf", fontSize);
	io.FontDefault = io.Fonts->AddFontFromFileTTF("res/fonts/opensans/OpenSans-Regular.ttf", fontSize);

	ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

	SetDarkThemeColors();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

	Camera camera(width, height, glm::vec3(0.0f, 0.0f, 5.0f));

	Scene scene(camera);

	Shader::Init();
	Texture::Init();
	RasterizerRenderer rasterizerRenderer;
	RayTracerRenderer raytracer;

	Application app(camera, scene, *openglRenderer, rasterizerRenderer, raytracer);

	openglRenderer->GenerateCubemapFromEquirectangular(scene);
	openglRenderer->GenerateIrradianceMap(scene);
	openglRenderer->GeneratePrefilterMap(scene);
	openglRenderer->GenerateBrdfLUT(scene);

	double lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
    	double nowTime = glfwGetTime();
		double deltaTime = nowTime - lastTime;
		lastTime = nowTime;

    	camera.SceneInputs(window, (float)deltaTime);
		openglRenderer->Render(scene);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		app.DrawEditor();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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

	Texture::Uninit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}