#pragma once
#include "matrix.h"

inline mat4 Translate(const vec3& v)
{
    return mat4(
        1.0f, 0.0f, 0.0f, v.x,
        0.0f, 1.0f, 0.0f, v.y,
        0.0f, 0.0f, 1.0f, v.z,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

inline mat4 Scale(float scale)
{
    return mat4(
        scale, 0, 0, 0,
        0, scale, 0, 0,
        0, 0, scale, 0,
        0, 0, 0, 1
    );
}

inline mat4 Scale(const vec3& v)
{
    return mat4(
        v.x, 0, 0, 0,
        0, v.y, 0, 0,
        0, 0, v.z, 0,
        0, 0, 0, 1
    );
}

inline mat4 RotateY(float angle)
{
    return mat4(
        cosf(angle), 0, sinf(angle), 0,
        0, 1, 0, 0,
        -sinf(angle), 0, cosf(angle), 0,
        0, 0, 0, 1
    );
}

inline mat4 Perspective(float fov, float aspect, float near, float far)
{
    float temp = 1.0f / tanf(0.5f * fov);
    float d = 1.0f / (far - near);
    return mat4(
        temp/aspect, 0.0f, 0.0f, 0.0f,
        0.0f, temp, 0.0f, 0.0f,
        0.0f, 0.0f, -(far+near) * d, -(2*far*near)*d,
        0.0f, 0.0f, -1.0f, 0.0f
    );
}
