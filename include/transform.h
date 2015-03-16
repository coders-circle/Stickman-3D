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

inline mat4 RotateX(float angle)
{
    return mat4(
        1, 0, 0, 0,
        0, cosf(angle), -sinf(angle), 0,
        0, sinf(angle), cosf(angle), 0,
        0, 0, 0, 1
    );
}

inline mat4 RotateZ(float angle)
{
    return mat4(
        cosf(angle), -sinf(angle), 0, 0,
        sinf(angle), cosf(angle), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}

inline mat4 EulerXYZ(const vec3& v)
{
    return RotateZ(v.z) * RotateY(v.y) * RotateX(v.x);
}

inline mat4 EulerZYX(const vec3& v)
{
    return RotateX(v.x) * RotateY(v.y) * RotateZ(v.z);
}

inline mat4 LookAt(const vec3& eye, const vec3& target, const vec3& up)
{
    vec3 zaxis = eye-target;
    vec3 xaxis = up.Cross(zaxis);
    zaxis.Normalize();
    xaxis.Normalize();
    vec3 yaxis = zaxis.Cross(xaxis);

    return mat4(
        vec4(xaxis, -eye.Dot(xaxis)),
        vec4(yaxis, -eye.Dot(yaxis)),
        vec4(zaxis, -eye.Dot(zaxis)),
        vec4(0,0,0,1)
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

inline mat4 Orthographic(float left, float right, float bottom, float top, float near, float far)
{
    float rl = 1.0f/(right - left);
    float tb = 1.0f/(top - bottom);
    float fn = 1.0f/(far - near);
    return mat4(
        2*rl, 0, 0, -(right+left)*rl,
        0, 2*tb, 0, -(top+bottom)*tb,
        0, 0, -2*fn, -(far+near)*fn,
        0, 0, 0, 1
    );
}
