#pragma once

// Each point denotes a window-space pixel position,
// depth of the pixel and varying attributes for the pixel
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
    
        // Create edges out of the points
        // But do not create horizontal edges
        // Edge stores two points sorted by y
        Edge<N> edges[3];
        int num = 0;
        if (pt1[1] != pt2[1])
            edges[num++].Initialize(point1, point2);
        if (pt2[1] != pt3[1])
            edges[num++].Initialize(point2, point3);
        if (pt3[1] != pt1[1])
            edges[num++].Initialize(point3, point1);
            
        // If only two edges are created (that is 3rd is horizontal)
        // draw spans from for the pair
        if (num == 2)
        {
            // Pair stores two edges sorted by x
            Pair<N> pair(&edges[0], &edges[1]);
            DrawSpans(pair, f, width, height, depthBuffer);
        }
        // If 3 edges were created, find the longest edge and draw spans for two pairs
        // each pair containing the longest edge and a sort edge
        else if (num == 3)
        {
            int le = 0;
            if (edges[1].dy > edges[0].dy)
                le = 1;
            if (edges[2].dy > edges[le].dy)
                le = 2;

            int se1 = (le+1)%3, se2 = (le+2)%3;
            if (edges[se2].y < edges[se1].y)        // Make sure short edge with less y is drawn first
                Swap(se1, se2);                     //  to ensure that the longest edge has its y scanned from top to bottom and not from middle

            Pair<N> p1(&edges[le], &edges[se1]), p2(&edges[le], &edges[se2]);
            DrawSpans(p1, f, width, height, depthBuffer);
            DrawSpans(p2, f, width, height, depthBuffer);
        }
    }

private:

    // Edge stores a pair of points, sorted by y-coordinate
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
        
        // Find x-coordinate for next y-coordinate
        // Also update the varying attributes and depth
        //  by interpolating along the edge
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
        
    // Pair stores a pair of edges sorted by x-coordinate 
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
        
        // Find x-coordinates for each edges for next y
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
        // To draw the spans
        //  start at first y of the pair of edges (which are equal at the start)
        //  get x's of each edges
        //  interpolate attributes and depth
        //  call fragmentShader function to draw the point
        //  find next Y
        // Do this till y's of edges exhaust or y is beyond the window height

        vec4 at_diffs[N];
        Point<N> point;
        float xdiff;
        float ddiff;
        while (true)
        {
            int y = p.e1->y;
            if (y >= h)         // Clipping when y >= height
                return;
            if (y >= 0)         // Clipping when y < 0
            {
                int x1 = p.e1->x;
                int x2 = p.e2->x;
                if (x1 < w && x2 >= 0)      // Clipping when x > width or x < 0
                {
                    x1 = Max(x1, 0);        // Further clipping
                    x2 = Min(x2, w);

                    point.pos[1] = y;
                    xdiff = p.e2->x - p.e1->x;
                    {
                        ddiff = p.e2->d - p.e1->d;
                        for (int i=0; i<N; ++i)
                            at_diffs[i] = p.e2->attrs[i] - p.e1->attrs[i];

                        // for each point as we scan interpolate depth and attributes
                        for (point.pos[0] = x1; point.pos[0] <= x2; ++point.pos[0])
                        {
                            float factor = float(point.pos[0]-p.e1->x)/xdiff;
                            point.d = p.e1->d + ddiff * factor;
                            // depth clipping (d < 0 and d > 1) Since depth buffer store 1 at max, d>1 is automatically tested
                            if (point.d < 0)
                                continue;
                            // Depth test
                            float& depth = depthBuffer[point.pos[1]*w+point.pos[0]];
                            if (depth < point.d)
                                continue;

                            for (int i=0; i<N; ++i)
                                point.varying[i] = p.e1->attrs[i] + at_diffs[i] * factor;
                            depth = point.d;
                            f(point);
                        }
                    }
                }
            }

           if (!p.NextY())
                return;
        }
    }

};
