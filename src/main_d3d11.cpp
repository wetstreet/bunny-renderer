#include "Application.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "d3d11/D3D11Renderer.h"

const unsigned int width = 1400;
const unsigned int height = 900;

std::unique_ptr<Camera> camera;

std::shared_ptr<D3D11Renderer> pD3D11Renderer;

void TextureRegister(Texture* tex) { pD3D11Renderer->RegisterTexture(tex); }
void SkyboxRegister(Skybox* skybox) { pD3D11Renderer->RegisterSkybox(skybox); }
void MeshRegister(Mesh* mesh) { pD3D11Renderer->RegisterMesh(mesh); }

void TextureUnregister(Texture* tex) { pD3D11Renderer->UnregisterTexture(tex); }
void MeshUnregister(Mesh* mesh) { pD3D11Renderer->UnregisterMesh(mesh); }
void SkyboxUnregister(Skybox* skybox) { pD3D11Renderer->UnregisterSkybox(skybox); }
void TextureBind(Texture& tex, GLuint slot) { pD3D11Renderer->BindTexture(tex, slot); }

// Main code
int main(int, char**)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    auto window = glfwCreateWindow(width, height, "glfw - DX12", nullptr, nullptr);
    if (!window) {
        std::cout << "Creat window error" << std::endl;
        return -1;
    }
    //glfwSetFramebufferSizeCallback(window, OnResizeFrame);

    auto hwnd = glfwGetWin32Window(window);

    TextureRegisterFunction = TextureRegister;
    SkyboxRegisterFunction = SkyboxRegister;
    MeshRegisterFunction = MeshRegister;

    TextureUnregisterFunction = TextureUnregister;
    SkyboxUnregisterFunction = SkyboxUnregister;
    MeshUnregisterFunction = MeshUnregister;
    TextureBindFunction = TextureBind;

    pD3D11Renderer = std::make_shared< D3D11Renderer>();

    D3D11Renderer& d3d11Renderer = *pD3D11Renderer;

    d3d11Renderer.Init(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplDX11_Init(d3d11Renderer.pd3dDevice, d3d11Renderer.pd3dDeviceContext);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    Camera camera(width, height, glm::vec3(0.0f, 0.0f, 5.0f));
    Scene scene(camera);
    /*
    Shader::Init();
    Texture::Init();*/

    RasterizerRenderer rasterizerRenderer;
    RayTracerRenderer raytracer;

    Application app(camera, scene, d3d11Renderer, rasterizerRenderer, raytracer);

    d3d11Renderer.InitRender();

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double nowTime = glfwGetTime();
        double deltaTime = nowTime - lastTime;
        lastTime = nowTime;

        camera.SceneInputs(window, (float)deltaTime);
        d3d11Renderer.Render(scene);

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        app.DrawEditor();

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        d3d11Renderer.SetRenderTarget(clear_color_with_alpha);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        d3d11Renderer.Present();

        glfwPollEvents();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    d3d11Renderer.CleanupDeviceD3D();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}