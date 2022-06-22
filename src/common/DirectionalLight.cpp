#include "DirectionalLight.h"

glm::vec3 DirectionalLight::GetLightPosition()
{
	glm::mat4 rotationMatrix = glm::mat4(1);

	for (int i = 0; i < 3; i++)
	{
		rotationMatrix = glm::rotate(rotationMatrix, rotation[i], directionUnary[i]);
	}

    return rotationMatrix * glm::vec4(0, 0, -1, 0);
}