#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <cstring>

enum Type
{
    Type_Mesh,
    Type_Light
};

class Object
{
    public:
        bool isEnabled = true;
        char name[32];
        glm::vec3 position = glm::vec3(0);
        glm::vec3 rotation = glm::vec3(0);
        glm::vec3 scale = glm::vec3(1);

        glm::mat4 objectToWorld = glm::mat4(1);

    public:
        virtual Type GetType() = 0;
        void UpdateMatrix();
        void SetName(const char *name);
};

static const float ZPI = 3.14159265358979323846f;
static const float RAD2DEG = (180.f / ZPI);
static const float DEG2RAD = (ZPI / 180.f);

static const glm::vec3 directionUnary[3] = { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) };

#endif //__OBJECT_H__