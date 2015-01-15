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

    template<int N, class... Args, class... Args1>
    void DrawTriangle(vec4(*f)(vec4[], Args...), void(*f1)(Point<N>&), Args1*... args)
    {
        Point<N> pt1, pt2, pt3;
        vec3 v1, v2, v3;
        // Vertex Shader...
        v1 = f(pt1.varying, args[0]...).ConvertToVec3();
        v2 = f(pt2.varying, args[1]...).ConvertToVec3();
        v3 = f(pt3.varying, args[2]...).ConvertToVec3();

        // Normalized x,y units to pixel units
        v1.x = (v1.x + 1.0f) / 2*m_width;
        v1.y = (-v1.y + 1.0f) / 2*m_height; 
        v2.x = (v2.x + 1.0f) / 2*m_width;
        v2.y = (-v2.y + 1.0f) / 2*m_height; 
        v3.x = (v3.x + 1.0f) / 2*m_width;
        v3.y = (-v3.y + 1.0f) / 2*m_height;

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
