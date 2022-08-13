#ifndef __CUBEMAP_H__
#define __CUBEMAP_H__

#include "Texture.h"

class Cubemap
{
public:
    GLuint ID;
    std::string name;
    std::string path;

    unsigned char** textures;
    int width, height, numColCh;
    Cubemap(std::string path);
    ~Cubemap();
};

#endif