#pragma once
#include "matrix.h"

class quat
{
public:
    struct { float x, y, z, w; };
    quat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    quat(float s = 0, float w = 1) : x(s), y(s), z(s), w(w) {}

    float Length() const
    {
        return sqrtf(x*x+y*y+z*z+w*w);
    }

    void Normalize()
    {
        float l = Length();
        if (l==0)
        {
            x = y = z = 0;
            w = 1;
            return;
        }
        x /= l;
        y /= l;
        z /= l;
        w /= l;
    }

    operator mat4() const
    {
        return mat4(
            1-2*y*y-2*z*z, 2*x*y-2*z*w, 2*x*z+2*y*w, 0,
            2*x*y+2*z*w, 1-2*x*x-2*z*z, 2*y*z-2*x*w, 0,
            2*x*z-2*y*w, 2*y*z+2*x*w, 1-2*x*x-2*y*y, 0,
            0, 0, 0, 1
        );
    }

    float& operator[] (size_t i)
    {
        return (&x)[i];
    }
    float operator[] (size_t i) const
    {
        return (&x)[i];
    }

    quat operator+(const quat &other) const
    {
#ifndef USE_SSE
        return quat(x+other.x, y+other.y, z+other.z, w+other.w);
#else
        return quat(_mm_add_ps(xyzw, other.xyzw));
#endif
    }
    quat operator-(const quat &other) const
    {
#ifndef USE_SSE
        return quat(x-other.x, y-other.y, z-other.z, w-other.w);
#else
        return quat(_mm_sub_ps(xyzw, other.xyzw));
#endif
    }
    quat operator*(const quat &other) const
    {
#ifndef USE_SSE
        return quat(x*other.x, y*other.y, z*other.z, w*other.w);
#else
        return quat(_mm_mul_ps(xyzw, other.xyzw));
#endif
    }
    quat operator*(float p) const
    {
#ifndef USE_SSE
        return quat(x*p, y*p, z*p, w*p);
#else
        return quat(_mm_mul_ps(xyzw, _mm_set1_ps(p)));
#endif
    }
    quat operator/(float p) const
    {
#ifndef USE_SSE
        return quat(x/p, y/p, z/p, w/p);
#else
        return quat(_mm_div_ps(xyzw, _mm_set1_ps(p)));
#endif
    }
    float Dot(const quat &other) const
    {
#ifndef USE_SSE
        return x*other.x + y*other.y + z*other.z + w*other.w;
#else
        return _mm_cvtss_f32(_mm_dp_ps(xyzw, other.xyzw, 0xF1));
#endif
    }
    quat operator -() const 
    {
        return quat(-x, -y, -z, -w);
    }
};
