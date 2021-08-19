#ifndef __SCENE_H__
#define __SCENE_H__

#include "Mesh.h"
#include "../common/Light.h"

class Scene
{
    public:
        Camera *camera;
        std::vector<Object*> objects;

        Scene(Camera *camera);
        ~Scene();

        Light *GetMainLight();
        void Draw();
        int AddObject(Object *object);
        void RemoveObject(int index);
};

#endif //__SCENE_H__