#ifndef __SCENE_H__
#define __SCENE_H__

#include "Mesh.h"
#include "Texture.h"
#include "../common/Light.h"

class Scene
{
    public:
        Camera *camera;
        std::vector<Object*> objects;
        Texture *head_diffuse;
        Texture *white_tex;

    public:
        Scene(Camera *camera);
        ~Scene();
        Light *GetMainLight();
        void Draw(Shader *shader);
        int AddObject(Object *object);
        void RemoveObject(int index);
        int AddPrimitive(std::string name);
};

#endif //__SCENE_H__