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
    union
    {
        struct { int pos[2]; };
        struct { int x, y; };
    };
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
        Point<N>* p1, *p2;        
        int tdx, tdy, dy, x, y, c;
        int xinc, yinc;

        float d, dincr;
        vec4 attrs[N], attrs_incr[N];
    
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
            d = 1/p1->d;
            dincr = (1/p2->d-1/p1->d)/float(dy);
            for (int i=0; i<N; ++i)
            {
                attrs[i] = p1->varying[i]*d;
                attrs_incr[i] = (p2->varying[i]/p2->d - p1->varying[i]*d)/float(dy);
            }
        }
        
        // Find x-coordinate for next y-coordinate
        // Also update the varying attributes and depth
        //  by interpolating along the edge
        bool NextY()
        {
            ++y;
            d += dincr;
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

        Point<N> point;
        float xdiff;

        float dincr, dr; vec4 attrs_incr[N], attrs_r[N];
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

                    dincr = (p.e2->d - p.e1->d)/float(xdiff);
                    dr = p.e1->d;
                    for (int i=0; i<N; ++i)
                    {
                        attrs_incr[i] = (p.e2->attrs[i] - p.e1->attrs[i])/float(xdiff);
                        attrs_r[i] = p.e1->attrs[i];
                        point.varying[i] = attrs_r[i] / dr;
                    }

                    // for each point as we scan interpolate depth and attributes
                    for (point.pos[0] = x1; point.pos[0] <= x2; ++point.pos[0])
                    {
                        // depth clipping (d < 0 and d > 1) Since depth buffer store 1 at max, d>1 is automatically tested
                        point.d = 1/dr;
                        if (point.x > 0)
                        {
                            // Depth test
                            float& depth = depthBuffer[point.pos[1]*w+point.pos[0]];
                            if (point.d < depth)
                            {
                                depth = point.d;
                                // Pass to the fragment shader
                                f(point);
                            }
                        }   
                        // Increment the depth and attributes
                        dr += dincr;
                        for (int i=0; i<N; ++i)
                        {
                            attrs_r[i] = attrs_r[i] + attrs_incr[i];
                            point.varying[i] = attrs_r[i] / dr;
                        }
                    }
                }
            }

           if (!p.NextY())
                return;
        }
    }

};
