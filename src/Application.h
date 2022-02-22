#include "imgui/imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "opengl/Scene.h"
#include "opengl/Texture.h"

#include "OpenGLRenderer.h"
#include "RasterizerRenderer.h"
#include "common/DirectionalLight.h"

#include "common/Utils.h"


class Application
{
public:
	Application()
	{
		strcpy(customMeshName, "");
	}

	void DrawEditor(Camera* camera, Scene& scene, ImGuiIO& io, OpenGLRenderer& openglRenderer, RasterizerRenderer& rasterizerRenderer);

	void DrawArrow(ImVec2 origin, ImVec2 worldDirSSpace, ImDrawList* drawList, ImU32 color);

private:
	ImVec2 viewport = ImVec2(800, 800);
	ImVec2 windowPos;

	int node_clicked = -1;

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

	bool mUsing = false;
};