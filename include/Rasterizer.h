#pragma once
#include "RasterizerStructs.h"

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
        Edge<N> edges[3];
        int num = 0;
        if (pt1[1] != pt2[1])
            edges[num++].Initialize(point1, point2);
        if (pt2[1] != pt3[1])
            edges[num++].Initialize(point2, point3);
        if (pt3[1] != pt1[1])
            edges[num++].Initialize(point3, point1);
            
        // If only two edges are created (that is 3rd is horizontal)
        //  draw spans from for thar one pair
        if (num == 2)
        {
            Pair<N> pair(&edges[0], &edges[1]);
            DrawSpans(pair, f, width, height, depthBuffer);
        }
        // If 3 edges were created, find the longest edge and draw spans for two pairs
        //  each pair containing the longest edge and a short edge
        else if (num == 3)
        {
            int le = 0;
            if (edges[1].dy > edges[0].dy)
                le = 1;
            if (edges[2].dy > edges[le].dy)
                le = 2;

            int se1 = (le+1)%3, se2 = (le+2)%3;
            if (edges[se2].y < edges[se1].y)        // Make sure short edge with less y is drawn first
                Swap(se1, se2);

            Pair<N> p1(&edges[le], &edges[se1]), p2(&edges[le], &edges[se2]);
            DrawSpans(p1, f, width, height, depthBuffer);
            DrawSpans(p2, f, width, height, depthBuffer);
        }
    }

private:
    template<int N>
    static void DrawSpans(Pair<N> &p, void(*f)(Point<N>&), int width, int height, float* depthBuffer)
    {
        Point<N> point;
        float xdiff;
        int start;

        float dincr; vec4 attrs_incr[N+1], attrs_tmp[N+1];
        float w, wincr;
        while (true)
        {
            int y = p.e1->y;
            if (y >= height)         // Clipping when y >= height
                return;
            if (y >= 0)         // Clipping when y < 0
            {
                int x1 = p.e1->x;
                int x2 = p.e2->x;
                if (x1 < width && x2 >= 0 && x1 < x2)      // Clipping when x > width or x < 0
                {
                    x1 = Max(x1, 0);        // Further clipping
                    x2 = Min(x2, width-1);

                    point.pos[1] = y;
                    xdiff = float(p.e2->x - p.e1->x);
                    start = x1 - p.e1->x;

                    dincr = (p.e2->d - p.e1->d)/float(xdiff);
                    point.d = p.e1->d + start*dincr;
                    wincr = (p.e2->w - p.e1->w)/float(xdiff);
                    w = p.e1->w + start*wincr;
                    for (int i=0; i<N; ++i)
                    {
                        attrs_incr[i] = (p.e2->attrs[i] - p.e1->attrs[i])/float(xdiff);
                        attrs_tmp[i] = p.e1->attrs[i] + attrs_incr[i] * start;
                        point.attribute[i] = attrs_tmp[i]/w;
                    }

                    for (point.pos[0] = x1; point.pos[0] <= x2; ++point.pos[0])
                    {
                        // depth clipping (d < 0 and d > 1) Since depth buffer store 1 at max, d>1 is automatically tested
                        if (point.d > 0)
                        {
                            // Depth test
                            float& depth = depthBuffer[point.pos[1]*width+point.pos[0]];
                            if (point.d < depth)
                            {
                                depth = point.d;
                                // Pass to the fragment shader
                                f(point);
                            }
                        }   
                        // Increment the depth and attributes
                        point.d += dincr;
                        w += wincr;
                        for (int i=0; i<N; ++i)
                        {
                            attrs_tmp[i] = attrs_tmp[i] + attrs_incr[i];
                            point.attribute[i] = attrs_tmp[i]/w;
                        }
                    }
                }
            }

           if (!p.NextY())
                return;
        }
    }

};
