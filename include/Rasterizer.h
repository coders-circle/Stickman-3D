#pragma once

template<int N>
class Point
{
public:
    Point(int x=0, int y=0)
    :d(0.0f)
    {
        pos[0]=x;
        pos[1]=y;
    }
    void FromVec3(const vec3& v)
    {
        pos[0] = v.x;
        pos[1] = v.y;
        d = v.z;
    }
    int pos[2];
    float d;
    vec4 varying[N];
};

class Rasterizer
{
public:
    template<int N>
    static void DrawTriangle(Point<N>* point1, Point<N>* point2, Point<N>* point3, void(*f)(Point<N>&), int width, int height, float* depthBuffer)
    {
        int *pt1 = point1->pos,
            *pt2 = point2->pos,
            *pt3 = point3->pos;
#define Clip(x) (x->pos[0] < 0 || x->pos[0] > width || x->pos[1] < 0 || x->pos[1] > height || x->d < 0 || x->d > 1)
        if (Clip(point1) && Clip(point2) && Clip(point3))
            return;
#undef Clip
        Edge<N> edges[3];
        int num = 0;
        if (pt1[1] != pt2[1])
            edges[num++].Initialize(point1, point2);
        if (pt2[1] != pt3[1])
            edges[num++].Initialize(point2, point3);
        if (pt3[1] != pt1[1])
            edges[num++].Initialize(point3, point1);
        
        if (num == 2)
        {
            Pair<N> pair(&edges[0], &edges[1]);
            DrawSpans(pair, f, width, height, depthBuffer);
        }
        else
        {
            int le = 0;
            if (edges[1].dy > edges[0].dy)
                le = 1;
            if (edges[2].dy > edges[le].dy)
                le = 2;
            int se1 = (le+1)%3, se2 = (le+2)%3;
            if (edges[se2].y < edges[se1].y)
                Swap(se1, se2);
            Pair<N> p1(&edges[le], &edges[se1]), p2(&edges[le], &edges[se2]);
            DrawSpans(p1, f, width, height, depthBuffer);
            DrawSpans(p2, f, width, height, depthBuffer);
        }
    }

private:
    template<int N>
    class Edge
    {
public:
        int x2, tdx, tdy, dy, x, y, y2, c;
        int xinc, yinc;

        float d, ddiff;
        Point<N>* initial;        
        vec4 at_diffs[N];
        vec4 attrs[N];
    
        void Initialize(Point<N>* point1, Point<N>* point2)
        {
            if (point1->pos[1] > point2->pos[1])
                Swap(point1, point2);
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
            x2 = pt2[0];
            y = pt1[1];
            y2 = pt2[1];
            
            initial = point1;
            d = point1->d;
            ddiff = point2->d - point1->d;
            for (int i=0; i<N; ++i)
            {
                at_diffs[i] = point2->varying[i] - point1->varying[i];
                attrs[i] = point1->varying[i];
            }
        }
    
        bool NextY()
        {
            ++y;
            if (y > y2)
                return false;
            
            float f = float(y-initial->pos[1])/float(dy);
            d = initial->d + ddiff * f;
            for (int i=0; i<N; ++i)
                attrs[i] = initial->varying[i] + at_diffs[i]*f;
            c += tdx;
            while (c >= dy)
            {
                x += xinc;
                c -= tdy;
            }
            return true;
        }
    };
    
    template<int N>
    class Pair
    {
    public:
        Edge<N> *e1, *e2;
        Pair(Edge<N> *_e1, Edge<N>*_e2)
        : e1(_e1), e2(_e2)
        {
              if (e1->initial->pos[0] > e2->initial->pos[0])
                Swap(e1, e2);
              
        }

        bool NextY()
        {    
            if (!e1->NextY() || !e2->NextY())
                return false;
            if (e1->x > e2->x)
                Swap(e1, e2);
            return true;
        }
    };
    
    template<int N>
    static void DrawSpans(Pair<N> &p, void(*f)(Point<N>&), int w, int h, float* depthBuffer)
    {
        vec4 at_diffs[N];
        Point<N> point;
        float xdiff;
        float ddiff;
        while (true)
        {
            int y = p.e1->y;
            if (y >= h)
                return;
            if (y >= 0)
            {
                int x1 = p.e1->x;
                int x2 = p.e2->x;
                if (x1 < w && x2 >= 0)
                {
                    x1 = Max(x1, 0);
                    x2 = Min(x2, w);

                    point.pos[1] = y;
                    xdiff = x2 - x1;
                    ddiff = p.e2->d - p.e1->d;
                    for (int i=0; i<N; ++i)
                        at_diffs[i] = p.e2->attrs[i] - p.e1->attrs[i];

                    for (point.pos[0] = x1; point.pos[0] <= x2; ++point.pos[0])
                    {
                        float factor = float(point.pos[0]-x1)/xdiff;
                        point.d = p.e1->d + ddiff * factor;
                        if (point.d < 0)
                            continue;
                        float& depth = depthBuffer[point.pos[1]*w+point.pos[0]];
                        if (depth < point.d)
                            continue;
                        depth = point.d;
                        for (int i=0; i<N; ++i)
                            point.varying[i] = p.e1->attrs[i] + at_diffs[i] * factor;
                        
                        f(point);
                    }
                }
            }

           if (!p.NextY())
                return;
        }
    }

};
