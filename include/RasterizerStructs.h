#pragma once

// Each point stores a window-space pixel position,
//  depth of the pixel and attributes for the pixel
template<int N>
class Point
{
public:
    Point(int x=0, int y=0, float d=0.0f)
    :d(d)
    {
        pos[0]=x;
        pos[1]=y;
    }
    void FromVec4(const vec4& v)
    {
        pos[0] = v.x;
        pos[1] = v.y;
        d = v.z;
        w = 1.0f/v.w;
    }
    union
    {
        struct { int pos[2]; };
        struct { int x, y; };
    };
    float d;
    vec4 attribute[N];
    float w;                // w is stored for perspective correct interpolation
};

// Edge stores a pair of points, sorted by y-coordinate
template<int N>
class Edge
{
public:
    Point<N>* p1, *p2;
    int tdx, tdy, dy, x, y, c; 
    int xinc, yinc;

    float d, dincr;
    vec4 attrs[N], attrs_incr[N];

    float w, wincr;

    void Initialize(Point<N>* point1, Point<N>* point2)
    {
        if (point1->pos[1] > point2->pos[1])
            Swap(point1, point2);
        p1 = point1;
        p2 = point2;
        int* pt1 = point1->pos;
        int* pt2 = point2->pos;
        tdx = 2*(pt2[0] - pt1[0]);
        if (tdx < 0)
        {
            xinc = -1;
            tdx = -tdx;
        }
        else 
            xinc = 1;
        dy = pt2[1] - pt1[1];
        tdy = 2*dy;
        c = 0;

        x = pt1[0];
        y = pt1[1];
        d = p1->d;
        dincr = (p2->d-p1->d)/float(dy);
        w = p1->w;
        wincr = (p2->w-p1->w)/float(dy);

        for (int i=0; i<N; ++i)
        {
            attrs[i] = p1->attribute[i]*p1->w;
            attrs_incr[i] = (p2->attribute[i]*p2->w - p1->attribute[i]*p1->w)/float(dy);
            /*
                Perspective correct interpolation of attributes is given as:
                    A = (A1/w1 + s(A2/w2-A1/w1))/(1/w)
                So we linearly interpolate from A1/w1 to A2/w2
                and at just at end, multiply by w
                Note: the "w" member of Point and Edge class stores 1/w instead of w
            */
        }
    }

    
    // Increment Y and update X, depth and attributes
    bool NextY()
    {
        ++y;
        d += dincr;
        w+= wincr;

        for (int i=0; i<N; ++i)
            attrs[i] = attrs[i] + attrs_incr[i];
        
        c += tdx;
        while (c >= dy)
        {
            x += xinc;
            c -= tdy;
        }
        if (y > p2->y)
            return false;
        return true;
    }
};
    
// Pair stores a pair of edges sorted by x-coordinate 
template<int N>
class Pair
{
public:
    Edge<N> *e1, *e2;
    Pair(Edge<N> *_e1, Edge<N>*_e2)
    : e1(_e1), e2(_e2)
    {
          if (e1->p1->pos[0] > e2->p1->pos[0])
            Swap(e1, e2);
          
    }
    
    // Get next Y for each edge
    bool NextY()
    {    
        if (!e1->NextY() || !e2->NextY())
            return false;
        if (e1->x > e2->x)
            Swap(e1, e2);
        return true;
    }
};

