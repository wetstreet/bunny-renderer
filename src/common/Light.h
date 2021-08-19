#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

class Light
{
    public:
        glm::vec3 position = glm::vec3(0);
        glm::vec3 rotation = glm::vec3(0);
        glm::vec3 scale = glm::vec3(1);

        float intensity = 1.0f;
        glm::vec3 color = glm::vec3(1, 1, 1);

        virtual glm::vec3 GetLightPosition() = 0;
};

static const float ZPI = 3.14159265358979323846f;
static const float RAD2DEG = (180.f / ZPI);
static const float DEG2RAD = (ZPI / 180.f);

static const glm::vec3 directionUnary[3] = { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) };

#endif //__LIGHT_H__