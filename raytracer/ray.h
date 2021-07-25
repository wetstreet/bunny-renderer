#ifndef __RAY_H__
#define __RAY_H__

#include "vec3.h"

class ray
{
    public:
    ray() {}
    ray(const point3 &origin, const vec3 &direction) : orig(origin), dir(direction) {}

    point3 origin() const { return orig; }
    vec3 direction() const { return dir; }

    point3 at(double t) const {
        return orig + dir * t;
    }
    public:
    point3 orig;
    vec3 dir;
};

#endif //__RAY_H__