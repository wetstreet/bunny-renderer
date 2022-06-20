#ifndef __SCENE_H__
#define __SCENE_H__

#include "Mesh.h"
#include "Texture.h"
#include "Light.h"
#include "Camera.h"

class Scene
{
    public:
        Camera &camera;
        std::vector<std::shared_ptr<Object>> objects;

    public:
        Scene(Camera &camera);
        ~Scene();
        std::shared_ptr<Light> GetMainLight();
        void Draw();
        int AddObject(std::shared_ptr<Object>);
        void RemoveObject(int index);
        int AddPrimitive(std::string name);
};

#endif //__SCENE_H__