#ifndef VECTORS_H
#define VECTORS_H
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdint.h>

typedef struct Vec3 {
    GLfloat x;
    GLfloat y;
    GLfloat z;
} Vec3;

Vec3 vec3Add(Vec3 a, Vec3 b);
Vec3 vec3Scale(Vec3 v, float s);

#endif