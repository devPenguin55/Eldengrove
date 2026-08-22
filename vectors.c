#include <GL/glu.h>
#include <GL/glut.h>
#include <stdint.h>
#include <math.h>
#include <assimp/matrix4x4.h>
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



Mat4 mat4Identity(void) {
    Mat4 result = {0};

    result.m[0][0] = 1.0f;
    result.m[1][1] = 1.0f;
    result.m[2][2] = 1.0f;
    result.m[3][3] = 1.0f;

    return result;
}

Mat4 mat4Multiply(Mat4 a, Mat4 b) {
    Mat4 result = {0};

    for (int row = 0; row < 4; row++) {
        for (int column = 0; column < 4; column++) {
            for (int i = 0; i < 4; i++) {
                result.m[row][column] +=
                    a.m[row][i] * b.m[i][column];
            }
        }
    }

    return result;
}

Mat4 mat4Translate(Vec3 position) {
    Mat4 result = mat4Identity();

    result.m[0][3] = position.x;
    result.m[1][3] = position.y;
    result.m[2][3] = position.z;

    return result;
}

Mat4 mat4Scale(Vec3 scale) {
    Mat4 result = mat4Identity();

    result.m[0][0] = scale.x;
    result.m[1][1] = scale.y;
    result.m[2][2] = scale.z;

    return result;
}

Mat4 mat4FromQuat(Quat rotation) {
    Mat4 result = mat4Identity();

    float x = rotation.x;
    float y = rotation.y;
    float z = rotation.z;
    float w = rotation.w;

    float xx = x * x;
    float yy = y * y;
    float zz = z * z;

    float xy = x * y;
    float xz = x * z;
    float yz = y * z;

    float wx = w * x;
    float wy = w * y;
    float wz = w * z;

    result.m[0][0] = 1.0f - 2.0f * (yy + zz);
    result.m[0][1] = 2.0f * (xy - wz);
    result.m[0][2] = 2.0f * (xz + wy);

    result.m[1][0] = 2.0f * (xy + wz);
    result.m[1][1] = 1.0f - 2.0f * (xx + zz);
    result.m[1][2] = 2.0f * (yz - wx);

    result.m[2][0] = 2.0f * (xz - wy);
    result.m[2][1] = 2.0f * (yz + wx);
    result.m[2][2] = 1.0f - 2.0f * (xx + yy);

    return result;
}

Mat4 mat4Inverse(Mat4 matrix) {
    Mat4 result;

    float *m = &matrix.m[0][0];
    float *inv = &result.m[0][0];

    inv[0] =
        m[5]  * m[10] * m[15] -
        m[5]  * m[11] * m[14] -
        m[9]  * m[6]  * m[15] +
        m[9]  * m[7]  * m[14] +
        m[13] * m[6]  * m[11] -
        m[13] * m[7]  * m[10];

    inv[4] =
        -m[4]  * m[10] * m[15] +
        m[4]  * m[11] * m[14] +
        m[8]  * m[6]  * m[15] -
        m[8]  * m[7]  * m[14] -
        m[12] * m[6]  * m[11] +
        m[12] * m[7]  * m[10];

    inv[8] =
        m[4]  * m[9] * m[15] -
        m[4]  * m[11] * m[13] -
        m[8]  * m[5] * m[15] +
        m[8]  * m[7] * m[13] +
        m[12] * m[5] * m[11] -
        m[12] * m[7] * m[9];

    inv[12] =
        -m[4]  * m[9] * m[14] +
        m[4]  * m[10] * m[13] +
        m[8]  * m[5] * m[14] -
        m[8]  * m[6] * m[13] -
        m[12] * m[5] * m[10] +
        m[12] * m[6] * m[9];

    inv[1] =
        -m[1]  * m[10] * m[15] +
        m[1]  * m[11] * m[14] +
        m[9]  * m[2] * m[15] -
        m[9]  * m[3] * m[14] -
        m[13] * m[2] * m[11] +
        m[13] * m[3] * m[10];

    inv[5] =
        m[0]  * m[10] * m[15] -
        m[0]  * m[11] * m[14] -
        m[8]  * m[2] * m[15] +
        m[8]  * m[3] * m[14] +
        m[12] * m[2] * m[11] -
        m[12] * m[3] * m[10];

    inv[9] =
        -m[0]  * m[9] * m[15] +
        m[0]  * m[11] * m[13] +
        m[8]  * m[1] * m[15] -
        m[8]  * m[3] * m[13] -
        m[12] * m[1] * m[11] +
        m[12] * m[3] * m[9];

    inv[13] =
        m[0]  * m[9] * m[14] -
        m[0]  * m[10] * m[13] -
        m[8]  * m[1] * m[14] +
        m[8]  * m[2] * m[13] +
        m[12] * m[1] * m[10] -
        m[12] * m[2] * m[9];

    inv[2] =
        m[1]  * m[6] * m[15] -
        m[1]  * m[7] * m[14] -
        m[5]  * m[2] * m[15] +
        m[5]  * m[3] * m[14] +
        m[13] * m[2] * m[7] -
        m[13] * m[3] * m[6];

    inv[6] =
        -m[0]  * m[6] * m[15] +
        m[0]  * m[7] * m[14] +
        m[4]  * m[2] * m[15] -
        m[4]  * m[3] * m[14] -
        m[12] * m[2] * m[7] +
        m[12] * m[3] * m[6];

    inv[10] =
        m[0]  * m[5] * m[15] -
        m[0]  * m[7] * m[13] -
        m[4]  * m[1] * m[15] +
        m[4]  * m[3] * m[13] +
        m[12] * m[1] * m[7] -
        m[12] * m[3] * m[5];

    inv[14] =
        -m[0]  * m[5] * m[14] +
        m[0]  * m[6] * m[13] +
        m[4]  * m[1] * m[14] -
        m[4]  * m[2] * m[13] -
        m[12] * m[1] * m[6] +
        m[12] * m[2] * m[5];

    inv[3] =
        -m[1]  * m[6] * m[11] +
        m[1]  * m[7] * m[10] +
        m[5]  * m[2] * m[11] -
        m[5]  * m[3] * m[10] -
        m[9]  * m[2] * m[7] +
        m[9]  * m[3] * m[6];

    inv[7] =
        m[0]  * m[6] * m[11] -
        m[0]  * m[7] * m[10] -
        m[4]  * m[2] * m[11] +
        m[4]  * m[3] * m[10] +
        m[8]  * m[2] * m[7] -
        m[8]  * m[3] * m[6];

    inv[11] =
        -m[0]  * m[5] * m[11] +
        m[0]  * m[7] * m[9] +
        m[4]  * m[1] * m[11] -
        m[4]  * m[3] * m[9] -
        m[8]  * m[1] * m[7] +
        m[8]  * m[3] * m[5];

    inv[15] =
        m[0]  * m[5] * m[10] -
        m[0]  * m[6] * m[9] -
        m[4]  * m[1] * m[10] +
        m[4]  * m[2] * m[9] +
        m[8]  * m[1] * m[6] -
        m[8]  * m[2] * m[5];

    float determinant =
        m[0] * inv[0] +
        m[1] * inv[4] +
        m[2] * inv[8] +
        m[3] * inv[12];

    if (fabsf(determinant) < 0.000001f) {
        return mat4Identity();
    }

    float inverseDeterminant = 1.0f / determinant;

    for (int i = 0; i < 16; i++) {
        inv[i] *= inverseDeterminant;
    }

    return result;
}

Vec3 vec3Lerp(Vec3 a, Vec3 b, float t) {
    Vec3 result;

    result.x = a.x + (b.x - a.x) * t;
    result.y = a.y + (b.y - a.y) * t;
    result.z = a.z + (b.z - a.z) * t;

    return result;
}

Quat quatSlerp(Quat a, Quat b, float t) {
    Quat result;

    float dot =
        a.x * b.x +
        a.y * b.y +
        a.z * b.z +
        a.w * b.w;

    // Make sure we take the shortest path.
    if (dot < 0.0f) {
        b.x = -b.x;
        b.y = -b.y;
        b.z = -b.z;
        b.w = -b.w;

        dot = -dot;
    }

    // If the quaternions are very close, use linear interpolation.
    if (dot > 0.9995f) {
        result.x = a.x + t * (b.x - a.x);
        result.y = a.y + t * (b.y - a.y);
        result.z = a.z + t * (b.z - a.z);
        result.w = a.w + t * (b.w - a.w);

        float length = sqrtf(
            result.x * result.x +
            result.y * result.y +
            result.z * result.z +
            result.w * result.w
        );

        if (length > 0.000001f) {
            result.x /= length;
            result.y /= length;
            result.z /= length;
            result.w /= length;
        }

        return result;
    }

    float theta = acosf(dot);
    float sinTheta = sinf(theta);

    float weightA = sinf((1.0f - t) * theta) / sinTheta;
    float weightB = sinf(t * theta) / sinTheta;

    result.x = a.x * weightA + b.x * weightB;
    result.y = a.y * weightA + b.y * weightB;
    result.z = a.z * weightA + b.z * weightB;
    result.w = a.w * weightA + b.w * weightB;

    return result;
}

Mat4 mat4FromAiMatrix4x4(struct aiMatrix4x4 matrix) {
    Mat4 result;

    result.m[0][0] = matrix.a1;
    result.m[0][1] = matrix.a2;
    result.m[0][2] = matrix.a3;
    result.m[0][3] = matrix.a4;

    result.m[1][0] = matrix.b1;
    result.m[1][1] = matrix.b2;
    result.m[1][2] = matrix.b3;
    result.m[1][3] = matrix.b4;

    result.m[2][0] = matrix.c1;
    result.m[2][1] = matrix.c2;
    result.m[2][2] = matrix.c3;
    result.m[2][3] = matrix.c4;

    result.m[3][0] = matrix.d1;
    result.m[3][1] = matrix.d2;
    result.m[3][2] = matrix.d3;
    result.m[3][3] = matrix.d4;

    return result;
}