#include "Object.h"

void Object::SetName(const char *name)
{
    strcpy(this->name, name);
}

void Object::UpdateMatrix()
{
	objectToWorld = glm::mat4(1);

	for (int i = 0; i < 3; i++)
	{
		objectToWorld = glm::rotate(objectToWorld, rotation[i] * DEG2RAD, directionUnary[i]);
	}

	float validScale[3];
	for (int i = 0; i < 3; i++)
	{
		if (fabsf(scale[i]) < FLT_EPSILON)
		{
		    validScale[i] = 0.001f;
		}
		else
		{
		    validScale[i] = scale[i];
		}
	}
	objectToWorld[0] *= validScale[0];
	objectToWorld[1] *= validScale[1];
	objectToWorld[2] *= validScale[2];
	objectToWorld[3] = glm::vec4(position[0], position[1], position[2], 1.f);
}