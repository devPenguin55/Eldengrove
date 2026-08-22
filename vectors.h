#ifndef VECTORS_H
#define VECTORS_H
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdint.h>
#include <assimp/matrix4x4.h>

typedef struct Vec3 {
    GLfloat x;
    GLfloat y;
    GLfloat z;
} Vec3;

typedef struct Quat {
    float x;
    float y;
    float z;
    float w;
} Quat;

typedef struct Mat4 {
    float m[4][4];
} Mat4;

Vec3 vec3Add(Vec3 a, Vec3 b);
Vec3 vec3Scale(Vec3 v, float s);
Mat4 mat4Identity(void);
Mat4 mat4Multiply(Mat4 a, Mat4 b);
Mat4 mat4Translate(Vec3 position);
Mat4 mat4Scale(Vec3 scale);
Mat4 mat4FromQuat(Quat rotation);
Mat4 mat4Inverse(Mat4 matrix);
Mat4 mat4FromAiMatrix4x4(struct aiMatrix4x4 matrix);

Vec3 vec3Lerp(Vec3 a, Vec3 b, float t);
Quat quatSlerp(Quat a, Quat b, float t);

#endif