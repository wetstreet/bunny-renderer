#ifndef __SCENE_H__
#define __SCENE_H__

#include "Mesh.h"

class Scene
{
    public:
        Camera *camera;
        std::vector<Mesh*> meshes;

        Scene(Camera *camera);

        void Draw() const;
        void AddMesh(Mesh *mesh);
        void Delete();
};

#endif //__SCENE_H__