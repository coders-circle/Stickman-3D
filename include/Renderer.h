#pragma once
#include "matrix.h"
#include "Rasterizer.h"
#include "Timer.h"


class Renderer
{
public:
    Renderer() : m_timer(/*60.0*/300.0) {}
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
    void SetClearColor(RGBColor clearColor) { m_clearColor = clearColor; }

    template<int N>
    void DrawTriangle(Point<N> &pt1, Point<N> &pt2, Point<N> &pt3, void (*f)(Point<N>&))
    {
        Rasterizer::DrawTriangle(&pt1, &pt2, &pt3, f, m_width, m_height); 
    }

    template<int N, class Args>
    void DrawTriangles(vec4(*vertexShader)(vec4[], const Args&), void(*fragmentShader)(Point<N>&), Args* vertexBuffer, uint16_t* indexBuffer, size_t numTriangles)
    {
        for (size_t i=0; i<numTriangles; ++i)
            DrawTriangle(vertexShader, fragmentShader, 
                        vertexBuffer[indexBuffer[i*3]], vertexBuffer[indexBuffer[i*3+1]], vertexBuffer[indexBuffer[i*3+2]]);
    }

    template<int N, class Args>
    void DrawTriangles(vec4(*vertexShader)(vec4[], const Args&), void(*fragmentShader)(Point<N>&), Args* vertexBuffer, size_t numTriangles)
    {
        for (size_t i=0; i<numTriangles; ++i)
            DrawTriangle(vertexShader, fragmentShader, vertexBuffer[i*3], vertexBuffer[i*3+1], vertexBuffer[i*3+2]);
    }

    template<int N, class Args>
    void DrawTriangle(vec4(*f)(vec4[], const Args&), void(*f1)(Point<N>&), Args &arg1, Args& arg2, Args& arg3)
    {
        Point<N> pt1, pt2, pt3;
        vec3 v1, v2, v3;
        // Vertex Shader...
        v1 = f(pt1.varying, arg1).ConvertToVec3();
        v2 = f(pt2.varying, arg2).ConvertToVec3();
        v3 = f(pt3.varying, arg3).ConvertToVec3();

        // Normalized x,y units to pixel units and depth (-1,1) to (0,1)
        v1.x = (v1.x + 1.0f) / 2*m_width;
        v1.y = (-v1.y + 1.0f) / 2*m_height; 
        v1.z = (v1.z + 1.0f) / 2.0f;
        v2.x = (v2.x + 1.0f) / 2*m_width;
        v2.y = (-v2.y + 1.0f) / 2*m_height; 
        v2.z = (v2.z + 1.0f) / 2.0f;
        v3.x = (v3.x + 1.0f) / 2*m_width;
        v3.y = (-v3.y + 1.0f) / 2*m_height;
        v3.z = (v3.z + 1.0f) / 2.0f;

        //std::cout << v1 << "\t" << v2 << "\t" << v3 << std::endl;
        
        // Rasterization which also performs clipping
        // and calls fragment shader for rendered pixels
        pt1.FromVec3(v1);
        pt2.FromVec3(v2);
        pt3.FromVec3(v3);
        DrawTriangle(pt1, pt2, pt3, f1);
    }

    int GetWidth() { return m_width; }
    int GetHeight() { return m_height; }

private:
    uint32_t* m_framebuffer;
    int m_width, m_height;
    Timer m_timer;
    
    SDL_Window* m_window;
    SDL_Surface* m_screen;

    void Clear()
    {
        for (int i = 0; i < m_width; ++i)
        for (int j = 0; j < m_height; ++j)
            PutPixelUnsafe(i, j, m_clearColor);
    }

    std::function<void()> m_render;
    std::function<void(double)> m_update;
    std::function<void(int, int)> m_resize;
    RGBColor m_clearColor;
};
