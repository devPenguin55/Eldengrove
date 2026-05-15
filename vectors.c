#include <GL/glu.h>
#include <GL/glut.h>
#include <stdint.h>
#include "vectors.h"

Vec3 vec3Add(Vec3 a, Vec3 b) {
    Vec3 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

Vec3 vec3Scale(Vec3 v, float s) {
    Vec3 result;
    result.x = v.x * s;
    result.y = v.y * s;
    result.z = v.z * s;
    return result;
}