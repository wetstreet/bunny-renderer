#include <iostream>

#include "imgui/imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui/imgui_internal.h"
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

void ScreenPosToWorldRay(
	int mouseX, int mouseY,             // Mouse position, in pixels, from bottom-left corner of the window
	int screenWidth, int screenHeight,  // Window size, in pixels
	glm::mat4 ViewMatrix,               // Camera position and orientation
	glm::mat4 ProjectionMatrix,         // Camera parameters (ratio, field of view, near and far planes)
	glm::vec3& out_origin,              // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
	glm::vec3& out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
){

	// The ray Start and End positions, in Normalized Device Coordinates (Have you read Tutorial 4 ?)
	glm::vec4 lRayStart_NDC(
		((float)mouseX/(float)screenWidth  - 0.5f) * 2.0f, // [0,1024] -> [-1,1]
		((float)mouseY/(float)screenHeight - 0.5f) * 2.0f, // [0, 768] -> [-1,1]
		-1.0, // The near plane maps to Z=-1 in Normalized Device Coordinates
		1.0f
	);
	glm::vec4 lRayEnd_NDC(
		((float)mouseX/(float)screenWidth  - 0.5f) * 2.0f,
		((float)mouseY/(float)screenHeight - 0.5f) * 2.0f,
		0.0,
		1.0f
	);


	// The Projection matrix goes from Camera Space to NDC.
	// So inverse(ProjectionMatrix) goes from NDC to Camera Space.
	glm::mat4 InverseProjectionMatrix = glm::inverse(ProjectionMatrix);
	
	// The View Matrix goes from World Space to Camera Space.
	// So inverse(ViewMatrix) goes from Camera Space to World Space.
	glm::mat4 InverseViewMatrix = glm::inverse(ViewMatrix);
	
	glm::vec4 lRayStart_camera = InverseProjectionMatrix * lRayStart_NDC;    lRayStart_camera/=lRayStart_camera.w;
	glm::vec4 lRayStart_world  = InverseViewMatrix       * lRayStart_camera; lRayStart_world /=lRayStart_world .w;
	glm::vec4 lRayEnd_camera   = InverseProjectionMatrix * lRayEnd_NDC;      lRayEnd_camera  /=lRayEnd_camera  .w;
	glm::vec4 lRayEnd_world    = InverseViewMatrix       * lRayEnd_camera;   lRayEnd_world   /=lRayEnd_world   .w;


	// Faster way (just one inverse)
	//glm::mat4 M = glm::inverse(ProjectionMatrix * ViewMatrix);
	//glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world/=lRayStart_world.w;
	//glm::vec4 lRayEnd_world   = M * lRayEnd_NDC  ; lRayEnd_world  /=lRayEnd_world.w;


	glm::vec3 lRayDir_world(lRayEnd_world - lRayStart_world);
	lRayDir_world = glm::normalize(lRayDir_world);


	out_origin = glm::vec3(lRayStart_world);
	out_direction = glm::normalize(lRayDir_world);
}


bool TestRayOBBIntersection(
	glm::vec3 ray_origin,        // Ray origin, in world space
	glm::vec3 ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
	glm::vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
	glm::vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
	glm::mat4 ModelMatrix,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
	float& intersection_distance // Output : distance between ray_origin and the intersection with the OBB
){
	
	// Intersection method from Real-Time Rendering and Essential Mathematics for Games
	
	float tMin = 0.0f;
	float tMax = 100000.0f;

	glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z);

	glm::vec3 delta = OBBposition_worldspace - ray_origin;

	// Test intersection with the 2 planes perpendicular to the OBB's X axis
	{
		glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);
		float e = glm::dot(xaxis, delta);
		float f = glm::dot(ray_direction, xaxis);

		if ( fabs(f) > 0.001f ){ // Standard case

			float t1 = (e+aabb_min.x)/f; // Intersection with the "left" plane
			float t2 = (e+aabb_max.x)/f; // Intersection with the "right" plane
			// t1 and t2 now contain distances betwen ray origin and ray-plane intersections

			// We want t1 to represent the nearest intersection, 
			// so if it's not the case, invert t1 and t2
			if (t1>t2){
				float w=t1;t1=t2;t2=w; // swap t1 and t2
			}

			// tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
			if ( t2 < tMax )
				tMax = t2;
			// tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
			if ( t1 > tMin )
				tMin = t1;

			// And here's the trick :
			// If "far" is closer than "near", then there is NO intersection.
			// See the images in the tutorials for the visual explanation.
			if (tMax < tMin )
				return false;

		}else{ // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
			if(-e+aabb_min.x > 0.0f || -e+aabb_max.x < 0.0f)
				return false;
		}
	}


	// Test intersection with the 2 planes perpendicular to the OBB's Y axis
	// Exactly the same thing than above.
	{
		glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);
		float e = glm::dot(yaxis, delta);
		float f = glm::dot(ray_direction, yaxis);

		if ( fabs(f) > 0.001f ){

			float t1 = (e+aabb_min.y)/f;
			float t2 = (e+aabb_max.y)/f;

			if (t1>t2){float w=t1;t1=t2;t2=w;}

			if ( t2 < tMax )
				tMax = t2;
			if ( t1 > tMin )
				tMin = t1;
			if (tMin > tMax)
				return false;

		}else{
			if(-e+aabb_min.y > 0.0f || -e+aabb_max.y < 0.0f)
				return false;
		}
	}


	// Test intersection with the 2 planes perpendicular to the OBB's Z axis
	// Exactly the same thing than above.
	{
		glm::vec3 zaxis(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z);
		float e = glm::dot(zaxis, delta);
		float f = glm::dot(ray_direction, zaxis);

		if ( fabs(f) > 0.001f ){

			float t1 = (e+aabb_min.z)/f;
			float t2 = (e+aabb_max.z)/f;

			if (t1>t2){float w=t1;t1=t2;t2=w;}

			if ( t2 < tMax )
				tMax = t2;
			if ( t1 > tMin )
				tMin = t1;
			if (tMin > tMax)
				return false;

		}else{
			if(-e+aabb_min.z > 0.0f || -e+aabb_max.z < 0.0f)
				return false;
		}
	}

	intersection_distance = tMin;
	return true;

}

ImVec2 GetScreenPos(glm::mat4 mat, glm::vec4 pos, ImVec2 start, ImVec2 size)
{
	glm::vec4 trans = mat * pos;

	trans *= 0.5f / trans.w; // perspective division
	trans += glm::vec4(0.5f, 0.5f, 0, 0); // convert [-1,1] to [0,1]

	trans.y = 1.f - trans.y; // opengl/directx diff

	trans.x *= size.x;
	trans.y *= size.y;
	trans.x += start.x;
	trans.y += start.y;
	return ImVec2(trans.x, trans.y);
}

void DrawArrow(ImVec2 origin, ImVec2 worldDirSSpace, ImDrawList *drawList, ImU32 color)
{
	ImVec2 dir(origin - worldDirSSpace);

	float d = sqrtf(ImLengthSqr(dir));
	dir /= d; // Normalize
	dir *= 6.0f;

	ImVec2 ortogonalDir(dir.y, -dir.x); // Perpendicular vector
	ImVec2 a(worldDirSSpace + dir);
	drawList->AddTriangleFilled(worldDirSSpace - dir, a + ortogonalDir, a - ortogonalDir, color);
}

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
			ImGui::Begin("Scene", &showScene);

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
					for(int i = 0; i < renderer.scene->objects.size(); i++)
					{
						Object* object = renderer.scene->objects[i];
						if (object->GetType() == Type_Mesh)
						{
							Mesh* mesh = (Mesh*)object;

							float intersection_distance; // Output of TestRayOBBIntersection()

							if ( TestRayOBBIntersection(
								ray_origin, 
								ray_direction, 
								mesh->minPos, 
								mesh->maxPos,
								mesh->objectToWorld,
								intersection_distance)
							){
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

				Object *object = renderer.scene->objects[node_clicked];

				ImDrawList *drawList = ImGui::GetWindowDrawList();

				glm::mat4 mat;

				bool local = false;
				if (local)
					mat = camera->cameraMatrix * object->objectToWorld; // local
				else
					mat = camera->cameraMatrix * glm::translate(object->position);
				
				ImVec2 origin = GetScreenPos(mat, glm::vec4(0, 0, 0, 1), windowPos, viewport);
				ImVec2 right = 	GetScreenPos(mat, glm::vec4(1, 0, 0, 1), windowPos, viewport);
				ImVec2 up = 	GetScreenPos(mat, glm::vec4(0, 1, 0, 1), windowPos, viewport);
				ImVec2 front = 	GetScreenPos(mat, glm::vec4(0, 0, 1, 1), windowPos, viewport);

				drawList->AddLine(origin, right, 	IM_COL32(255, 0, 0, 255), 3.f);
				drawList->AddLine(origin, up, 		IM_COL32(0, 255, 0, 255), 3.f);
				drawList->AddLine(origin, front, 	IM_COL32(0, 0, 255, 255), 3.f);
				
				DrawArrow(origin, right, 	drawList, IM_COL32(255, 0, 0, 255));
				DrawArrow(origin, up,		drawList, IM_COL32(0, 255, 0, 255));
				DrawArrow(origin, front, 	drawList, IM_COL32(0, 0, 255, 255));
				
				drawList->AddCircleFilled(origin, 6.f, IM_COL32(255, 255, 255, 255), 32);
				

				ImGui::EndChild();
			}
			

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