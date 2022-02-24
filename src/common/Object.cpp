#include "Object.h"
#include <glm/gtx/quaternion.hpp>

void Object::SetName(const char *name)
{
    strcpy(this->name, name);
}

void Object::UpdateMatrix()
{
	objectToWorld = glm::translate(glm::mat4(1.0f), position)
		* glm::toMat4(glm::quat(rotation))
		* glm::scale(glm::mat4(1.0f), scale);
}