#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "Object.h"

class Light : public Object
{
    public:
        float intensity = 1.0f;
        glm::vec3 color = glm::vec3(1, 1, 1);

        virtual glm::vec3 GetLightPosition() = 0;
        virtual Type GetType() { return Type_Light; };
};

#endif //__LIGHT_H__