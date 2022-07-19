#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>
#include <memory>
#include <iostream>

#include "imgui/imgui.h"
#include "glm/glm.hpp"

#define __DEVELOPMENT__ // use local resource path in development mode

std::string GetFileNameFromPath(std::string s);

std::string GetFileContents(std::string filename);

void ScreenPosToWorldRay(
	int mouseX, int mouseY,             // Mouse position, in pixels, from bottom-left corner of the window
	int screenWidth, int screenHeight,  // Window size, in pixels
	glm::mat4 ViewMatrix,               // Camera position and orientation
	glm::mat4 ProjectionMatrix,         // Camera parameters (ratio, field of view, near and far planes)
	glm::vec3& out_origin,              // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
	glm::vec3& out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
);


bool TestRayOBBIntersection(
	glm::vec3 ray_origin,        // Ray origin, in world space
	glm::vec3 ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
	glm::vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
	glm::vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
	glm::mat4 ModelMatrix,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
	float& intersection_distance // Output : distance between ray_origin and the intersection with the OBB
);

ImVec2 WorldToScreenPos(glm::mat4 mat, glm::vec4 pos, ImVec2 start, ImVec2 size);

glm::vec4 PointOnSegment(const glm::vec4& point, const glm::vec4& vertPos1, const glm::vec4& vertPos2);

glm::vec4 makeVect(const ImVec2 v);

bool DecomposeTransform(const glm::mat4& ModelMatrix, glm::vec3& Translation, glm::vec3& Rotation, glm::vec3& Scale);

std::ostream& operator<<(std::ostream& out, glm::vec4& v);

std::ostream& operator<<(std::ostream& out, glm::vec3& v);

std::ostream& operator<<(std::ostream& out, glm::vec2& v);

std::ostream& operator<<(std::ostream& out, ImVec2& v);

std::ostream& operator<<(std::ostream& out, glm::mat4& m);

#endif //__UTILS_H__