#include "Utils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"

std::string GetFileNameFromPath(std::string s)
{
	int slash = s.find_last_of('/');
	if (slash == -1)
		slash = s.find_last_of('\\');
	int dot = s.find_last_of('.');
	return s.substr(slash + 1, dot - slash - 1);
}

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

ImVec2 WorldToScreenPos(glm::mat4 mat, glm::vec4 pos, ImVec2 start, ImVec2 size)
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

glm::vec4 PointOnSegment(const glm::vec4& point, const glm::vec4& vertPos1, const glm::vec4& vertPos2)
{
	glm::vec4 c = point - vertPos1;
	glm::vec4 V = glm::normalize(vertPos2 - vertPos1);

	float d = glm::length(vertPos2 - vertPos1);
	float t = glm::dot(V, c);

	if (t < 0.f)
	{
		return vertPos1;
	}

	if (t > d)
	{
		return vertPos2;
	}

	return vertPos1 + V * t;
}

glm::vec4 makeVect(const ImVec2 v)
{
	return glm::vec4(v.x, v.y, 0, 0);
}

bool DecomposeTransform(const glm::mat4& ModelMatrix, glm::vec3& Translation, glm::vec3& Rotation, glm::vec3& Scale)
{
	// From glm::decompose in matrix_decompose.inl
	using namespace glm;
	using T = float;

	mat4 LocalMatrix(ModelMatrix);

	// Normalize the matrix.
	if (epsilonEqual(LocalMatrix[3][3], static_cast<T>(0), epsilon<T>()))
		return false;

	// First, isolate perspective.  This is the messiest.
	if (
		epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
		epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
		epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
	{
		// Clear the perspective partition
		LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
		LocalMatrix[3][3] = static_cast<T>(1);
	}

	// Next take care of translation (easy).
	Translation = vec3(LocalMatrix[3]);
	LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

	vec3 Row[3], Pdum3;

	// Now get scale and shear.
	for (length_t i = 0; i < 3; ++i)
		for (length_t j = 0; j < 3; ++j)
			Row[i][j] = LocalMatrix[i][j];

	Scale.x = length(Row[0]);
	Row[0] = detail::scale(Row[0], static_cast<T>(1));
	Scale.y = length(Row[1]);
	Row[1] = detail::scale(Row[1], static_cast<T>(1));
	Scale.z = length(Row[2]);
	Row[2] = detail::scale(Row[2], static_cast<T>(1));

#if 0
	// At this point, the matrix (in rows[]) is orthonormal.
	// Check for a coordinate system flip.  If the determinant
	// is -1, then negate the matrix and the scaling factors.
	Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
	if (dot(Row[0], Pdum3) < 0)
	{
		for (length_t i = 0; i < 3; i++)
		{
			Scale[i] *= static_cast<T>(-1);
			Row[i] *= static_cast<T>(-1);
		}
	}
#endif

	Rotation.y = asin(-Row[0][2]);
	 if (cos(Rotation.y) != 0) {
		 Rotation.x = atan2(Row[1][2], Row[2][2]);
		 Rotation.z = atan2(Row[0][1], Row[0][0]);
	 } else {
		 Rotation.x = atan2(-Row[2][0], Row[1][1]);
		 Rotation.z = 0;
	 }


	return true;
}

std::ostream &operator<<(std::ostream &out, glm::vec4 &v)
{
    out << "(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")";
    return out;
}

std::ostream &operator<<(std::ostream &out, glm::vec3 &v)
{
    out << "(" << v.x << "," << v.y << "," << v.z << ")";
    return out;
}

std::ostream &operator<<(std::ostream &out, glm::vec2 &v)
{
    out << "(" << v.x << "," << v.y << ")";
    return out;
}

std::ostream &operator<<(std::ostream &out, ImVec2 &v)
{
    out << "(" << v.x << "," << v.y << ")";
    return out;
}

std::ostream &operator<<(std::ostream &out, glm::mat4 &m)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            out << m[i][j] << " ";
        }
        out << std::endl;
    }
    return out;
}