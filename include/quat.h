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
};
