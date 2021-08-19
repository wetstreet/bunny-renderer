#ifndef __SCENE_H__
#define __SCENE_H__

#include "Mesh.h"
#include "../common/Light.h"

class Scene
{
    public:
        Camera *camera;
        std::vector<Mesh*> meshes;
        std::vector<Light*> lights;

        Scene(Camera *camera);
        Light *GetMainLight();
        void Draw();
        int AddMesh(Mesh *mesh);
        void RemoveMesh(int index);
        void Delete();
};

#endif //__SCENE_H__