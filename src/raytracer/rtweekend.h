#ifndef __RTWEEKEND_H__
#define __RTWEEKEND_H__

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    // Returns a random real in [0, 1)
    return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min, max)
    return min + (max - min) * random_double();
}

inline int random_int(int min, int max) {
    return static_cast<int>(random_double(min, max+1));
}

inline double clamp(double x, double min, double max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

#include "ray.h"
#include "vec3.h"

#endif //__RTWEEKEND_H__