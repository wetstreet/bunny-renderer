#ifndef __DIRECTIONAL_LIGHT_H__
#define __DIRECTIONAL_LIGHT_H__

#include "Light.h"

class DirectionalLight : public Light
{
    public:
        DirectionalLight()
        {
            SetName("directional light");
            position.y = 3;
            rotation.x = DEG2RAD * 50;
            rotation.y = DEG2RAD * -30;
        }
        virtual glm::vec3 GetLightPosition();
};

#endif //__DIRECTIONAL_LIGHT_H__