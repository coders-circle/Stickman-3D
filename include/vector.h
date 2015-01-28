#pragma once

template<class T>
inline void Swap(T &a, T &b)
{
    T t = a;
    a = b;
    b = t;
}

template<class T>
inline T Max(const T &a, const T &b)
{
    if (a > b)
        return a;
    return b;
}

template<class T>
inline T Min(const T &a, const T &b)
{
    if (a < b)
        return a;
    return b;
}

struct RGBColor
{
    RGBColor(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0)
    : r(red), g(green), b(blue)
    {}
    uint8_t b,g,r;
};

class vec2
{
public:
    union
    {
        struct { float u, v; };
        struct { float x, y; };
    };
    vec2() : x(0.0f), y(0.0f) {}
    vec2(float x, float y) : x(x), y(y) {}

    vec2 operator+(const vec2 &other) const
    {
        return vec2(x+other.x, y+other.y);
    }
    vec2 operator-(const vec2 &other) const
    {
        return vec2(x-other.x, y-other.y);
    }
    vec2 operator*(float p) const
    {
        return vec2(x*p, y*p);
    }
    vec2 operator/(float p) const
    {
        return vec2(x/p, y/p);
    }
    vec2 operator -() const 
    {
        return vec2(-x, -y);
    }
    float Dot(const vec2 &other) const
    {
        return x*other.x + y*other.y;
    }
    float& operator[] (size_t i)
    {
        return (&x)[i];
    }
    float operator[] (size_t i) const
    {
        return (&x)[i];
    }
    float Length()
    {
        return sqrtf(x*x+y*y);
    }
    void Normalize()
    {
        float l = Length();
        if (l==0)
        {
            x = 1; y = 0;
            return;
        }
        x /= l;
        y /= l;
    }
};


class vec3
{
public:
    union
    {
        struct { float r, g, b; };
        struct { float x, y, z; };
    };
    vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    vec3(const RGBColor &color) : r(color.r/255.0f), g(color.g/255.0f), b(color.b/255.0f) {}
    vec3(const vec2& v, float z=0.0f) : x(v.x), y(v.y), z(z) {}

    vec3 operator+(const vec3 &other) const
    {
        return vec3(x+other.x, y+other.y, z+other.z);
    }
    vec3 operator-(const vec3 &other) const
    {
        return vec3(x-other.x, y-other.y, z-other.z);
    }
    float Dot(const vec3 &other) const
    {
        return x*other.x + y*other.y + z*other.z;
    }
    vec3 operator*(float p) const
    {
        return vec3(x*p, y*p, z*p);
    }
    vec3 operator/(float p) const
    {
        return vec3(x/p, y/p, z/p);
    }
    vec3 operator -() const 
    {
        return vec3(-x, -y, -z);
    }
    float& operator[] (size_t i)
    {
        return (&x)[i];
    }
    float operator[] (size_t i) const
    {
        return (&x)[i];
    }
    float Length()
    {
        return sqrtf(x*x+y*y+z*z);
    }
    void Normalize()
    {
        float l = Length();
        if (l==0)
        {
            x=1;y=0;z=0;
            return;
        }
        x /= l;
        y /= l;
        z /= l;
    }

    vec3 Cross(const vec3& v) const
    {
        return vec3(y*v.z - v.y*z, z*v.x - v.z*x, x*v.y - v.x*y);
    }

    operator vec2() const
    {
        return vec2(x, y);
    }


    operator RGBColor() const
    {
        return RGBColor(uint8_t(r*255), uint8_t(g*255), uint8_t(b*255));
    }
};

class vec4
{
public:
    union
    {
        struct { float r, g, b, a; };
        struct { float x, y, z, w; };
    };
    vec4() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
    vec4(float x, float y, float z, float w = 1.0f) : x(x), y(y), z(z), w(w) {}
    vec4(const vec3& v, float w=1.0f) : x(v.x), y(v.y), z(v.z), w(w) {}
    vec4(const vec2& v, float z=0.0f, float w=1.0f) : x(v.x), y(v.y), z(z), w(w) {}

    vec4 operator+(const vec4 &other) const
    {
        return vec4(x+other.x, y+other.y, z+other.z, w+other.w);
    }
    vec4 operator-(const vec4 &other) const
    {
        return vec4(x-other.x, y-other.y, z-other.z, w-other.w);
    }
    vec4 operator*(float p) const
    {
        return vec4(x*p, y*p, z*p, w*p);
    }
    vec4 operator/(float p) const
    {
        return vec4(x/p, y/p, z/p, w/p);
    }
    float Dot(const vec4 &other) const
    {
        return x*other.x + y*other.y + z*other.z + w*other.w;
    }
    vec4 operator -() const 
    {
        return vec4(-x, -y, -z, -w);
    }
    vec3 ConvertToVec3() const
    {
        return vec3(x/w, y/w, z/w);
    }
    float& operator[] (size_t i)
    {
        return (&x)[i];
    }
    float operator[] (size_t i) const
    {
        return (&x)[i];
    }
    operator vec3() const
    {
        return vec3(x, y, z);
    }
    operator vec2() const
    {
        return vec2(x, y);
    }
    float Length()
    {
        return sqrtf(x*x+y*y+z*z+w*w);
    }
    void Normalize()
    {
        float l = Length();
        if (l==0)
        {
            x=0;y=0;z=0;w=1;
            return;
        }
        x /= l;
        y /= l;
        z /= l;
        w /= l;
    }

};

inline std::ostream& operator << (std::ostream &os, const vec3 &r) 
{
    os << "X: " << r.r << " Y: " << r.g << " Z: " << r.b;
    return os;
}

inline std::ostream& operator << (std::ostream &os, const vec4 &r) 
{
    os << "X: " << r.r << " Y: " << r.g << " Z: " << r.b << " W: " << r.a;
    return os;
}



