#include "imgui/imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "ImGuizmo/ImGuizmo.h"

#include "opengl/Scene.h"
#include "opengl/Texture.h"
#include "opengl/Selection.h"

#include "OpenGLRenderer.h"
#include "RasterizerRenderer.h"
#include "common/DirectionalLight.h"

#include "common/Utils.h"


class Application
{
public:
	Application(Camera &camera, Scene &scene, ImGuiIO &io, OpenGLRenderer &openglRenderer, RasterizerRenderer &rasterizerRenderer)
		: camera(camera), scene(scene), io(io), openglRenderer(openglRenderer), rasterizerRenderer(rasterizerRenderer)
	{
		strcpy(customMeshName, "");
	}

	void DrawEditor();

	void DrawMenu();
	void DrawInspector();
	void DrawScene();
	void DrawGizmo();
	void DrawHierarchy();
	void DrawSceneCamera();
	void DrawRasterizer();
	void DrawImage();
	void ClickToSelect();

private:
	Camera &camera;
	Scene &scene;
	ImGuiIO &io;
	OpenGLRenderer &openglRenderer;
	RasterizerRenderer &rasterizerRenderer;

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

	bool postprocess = false;

	// special window
	bool showImage = false;
	bool showCustomMeshPopup = false;
	char customMeshName[32];
};