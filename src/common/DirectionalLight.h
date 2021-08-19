#ifndef __DIRECTIONAL_LIGHT_H__
#define __DIRECTIONAL_LIGHT_H__

#include "Light.h"

class DirectionalLight : public Light
{
    public:
        virtual glm::vec3 GetLightPosition();
};

#endif //__DIRECTIONAL_LIGHT_H__