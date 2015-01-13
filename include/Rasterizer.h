#pragma once

template<int N>
class Point
{
public:
    Point(int x=0, int y=0)
    {
        pos[0]=x;
        pos[1]=y;
    }
    int pos[2];
    vec4 varying[N];
};

class Rasterizer
{
public:
    template<int N>
    static void DrawTriangle(Point<N>* point1, Point<N>* point2, Point<N>* point3, void(*f)(Point<N>&))
    {
        int *pt1 = point1->pos,
            *pt2 = point2->pos,
            *pt3 = point3->pos;
        
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
            DrawSpans(pair, f);
        }
        else
        {
            int le = 0;
            if (edges[1].dy > edges[0].dy)
                le = 1;
            if (edges[2].dy > edges[le].dy)
                le = 2;
            int se1 = (le+1)%3, se2 = (le+2)%3;
            Pair<N> p1(&edges[le], &edges[se1]), p2(&edges[le], &edges[se2]);
            DrawSpans(p1, f);
            DrawSpans(p2, f);
        }
    }

private:
    template<int N>
    class Edge
    {
public:
        int x2, tdx, tdy, dy, x, y, y2, c;
        int xinc, yinc;

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
        Pair(Edge<N> *e1, Edge<N>*e2)
        : e1(e1), e2(e2)
        {
            if (e1->initial->pos[0] > e2->initial->pos[0] || e1->initial->pos[0] > e2->initial->pos[0])
                Swap(e1, e2);
        }
    };
    
    template<int N>
    static void DrawSpans(Pair<N> &p, void(*f)(Point<N>&))
    {
        vec4 at_diffs[N];
        Point<N> point;
        float xdiff;
        while (true)
        {
            point.pos[1] = p.e1->y;

            xdiff = p.e2->x - p.e1->x;
            for (int i=0; i<N; ++i)
                at_diffs[i] = p.e2->attrs[i] - p.e1->attrs[i];

            for (point.pos[0] = p.e1->x; point.pos[0] <= p.e2->x; ++point.pos[0])
            {
                float factor = float(point.pos[0]-p.e1->x)/xdiff;
                for (int i=0; i<N; ++i)
                    point.varying[i] = p.e1->attrs[i] + at_diffs[i] * factor;
                
                f(point);
            }

            if (!p.e1->NextY())
                return;
            if (!p.e2->NextY())
                return;
        }
    }

};
