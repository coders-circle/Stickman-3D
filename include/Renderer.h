#pragma once
#include "matrix.h"
#include "Rasterizer.h"
#include "Timer.h"

// Renderer responsible for drawing pixels and triangles
// and managing the window
class Renderer
{
public:
    Renderer();
    ~Renderer();

    void Initialize(const char* title, int x, int y, int width, int height);
    void MainLoop();
    void CleanUp();
    
    void PutPixel(int x, int y, RGBColor color)
    {
        if (y > m_height || x > m_width)
            return;
        m_framebuffer[y*m_width + x] = (0xFF << 24) | (color.r << 16) | (color.g << 8) | color.b;
    }
    // (Maybe) faster due to no validity check
    void PutPixelUnsafe(int x, int y, RGBColor color)
    {
        m_framebuffer[y*m_width + x] = (0xFF << 24) | (color.r << 16) | (color.g << 8) | color.b;
    }

    void SetRenderCallback(std::function<void()> renderCallback) { m_render = renderCallback; }
    void SetUpdateCallback(std::function<void(double)> updateCallback) { m_update = updateCallback; }
    void SetResizeCallback(std::function<void(int, int)> resizeCallback) { m_resize = resizeCallback; }

    // Draw a triangle from 3 vertices; the points are pixel points and the function
    //  is fragment shader called for each intermediate pixels
    template<int N>
    void DrawTriangle(Point<N> &pt1, Point<N> &pt2, Point<N> &pt3, void (*f)(Point<N>&))
    {
        Rasterizer::DrawTriangle(&pt1, &pt2, &pt3, f, m_width, m_height, m_depthBuffer); 
    }
    
    // Draw triangles with given vertices and indices
    //  The vertices are passed through the vertexShader function
    //  and rasterized. Each pixel is then passed through the framentShader function
    template<int N, class Args>
    void DrawTriangles(vec4(*vertexShader)(vec4[], const Args&), void(*fragmentShader)(Point<N>&), Args* vertexBuffer, size_t numVertices, uint16_t* indexBuffer, size_t numTriangles)
    {
        vec4* vs = new vec4[numVertices];                // array to carry the clip-space vertices after passed through vertexBuffer
        Point<N>* points = new Point<N>[numVertices];    // array to carry window space points and varyings for each vertex

        ProcessVertices(points, vs, vertexShader, vertexBuffer, numVertices);
        // Draw each triangle
        for (size_t i=0; i<numTriangles; ++i)
        {
            size_t i1 = indexBuffer[i*3], i2 = indexBuffer[i*3+1], i3 = indexBuffer[i*3+2];

            // Clip-Space clipping
            // Donot draw if none of the 3 points satisfy -w < x < w, -w < y < w and -w < z < w
#define Clip(a) (a.x < -a.w || a.x > a.w || a.y < -a.w || a.y > a.w || a.z < -a.w || a.z > a.w)
            if (Clip(vs[i1]) && Clip(vs[i2]) && Clip(vs[i3]))
                continue;
#undef Clip

            // Back-Face Culling
            // Use the clip-space vertices to cull back-faces and only draw front-faces
            float C =   -vs[i1].x*(vs[i2].y*vs[i3].z - vs[i3].y*vs[i2].z)
                        -vs[i2].x*(vs[i3].y*vs[i1].z - vs[i1].y*vs[i3].z)
                        -vs[i3].x*(vs[i1].y*vs[i2].z - vs[i2].y*vs[i1].z);
            // Triangle is back-face if normal of triangle has z-component(C) <= 0
            if (C > 0)  
               DrawTriangle(points[i1], points[i2], points[i3], fragmentShader);
        }
        delete[] points;
        delete[] vs;
    }
        
    // Process each vertex through vertexShader and
    // fill 'newVertices' with resulting clip-space vertices
    // and 'points' with corresponding window-space vertices as well as varyings
    template<int N, class Args>
    void ProcessVertices(Point<N>*points, vec4* newVertices, vec4(*f)(vec4[], const Args&), Args* args, size_t numVertices)
    {
        vec3 v;
        for (size_t i=0; i<numVertices; ++i)
        {
            newVertices[i] = f(points[i].varying, args[i]);
            for (int j=0; j<N; ++j)
                points[i].varying[j] = points[i].varying[j];
            v = newVertices[i].ConvertToVec3();
            v.x = (v.x + 1.0f) / 2*m_width;
            v.y = (-v.y + 1.0f) / 2*m_height;
            v.z = (v.z + 1.0f) / 2.0f;
            points[i].FromVec3(v);
        }
    }
    
    int GetWidth() { return m_width; }
    int GetHeight() { return m_height; }
    void SetClearColor(RGBColor clearColor) { m_clearColor = clearColor; }

private:
    uint32_t* m_framebuffer;
    int m_width, m_height;
    Timer m_timer;
    
    SDL_Window* m_window;
    SDL_Surface* m_screen;
    float* m_depthBuffer;
    
    // Clear the color-buffer and the depth-buffer
    void Clear()
    {
        for (int i = 0; i < m_width; ++i)
        for (int j = 0; j < m_height; ++j)
        {
            PutPixelUnsafe(i, j, m_clearColor); // Clear the color buffer
            m_depthBuffer[j*m_height+i] = 1.0f; // Clear the depth buffer
        }
    }

    std::function<void()> m_render;
    std::function<void(double)> m_update;
    std::function<void(int, int)> m_resize;
    RGBColor m_clearColor;
};
