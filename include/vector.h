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
    uint8_t r, g, b;
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
    vec3(const RGBColor &color) : r(color.r), g(color.g), b(color.b) {}

    vec3 operator+(const vec3 &other) const
    {
        return vec3(x+other.x, y+other.y, z+other.z);
    }
    vec3 operator-(const vec3 &other) const
    {
        return vec3(r-other.r, g-other.g, b-other.b);
    }
    vec3 operator*(float p) const
    {
        return vec3(r*p, g*p, b*p);
    }
    vec3 operator/(float p) const
    {
        return vec3(r/p, g/p, b/p);
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
        x /= l;
        y /= l;
        z /= l;
    }


    operator RGBColor() const
    {
        return RGBColor(r*255, g*255, b*255);
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
    vec4(const vec3& v) : x(v.x), y(v.y), z(v.z), w(1.0f) {}

    vec4 operator+(const vec4 &other) const
    {
        return vec4(x+other.x, y+other.y, z+other.z, w+other.w);
    }
    vec4 operator-(const vec4 &other) const
    {
        return vec4(r-other.r, g-other.g, b-other.b, a-other.a);
    }
    vec4 operator*(float p) const
    {
        return vec4(r*p, g*p, b*p, a*p);
    }
    vec4 operator/(float p) const
    {
        return vec4(r/p, g/p, b/p, a/p);
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
        return vec3(r, g, b);
    }

    float Length()
    {
        return sqrtf(x*x+y*y+z*z+w*w);
    }

    void Normalize()
    {
        float l = Length();
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



