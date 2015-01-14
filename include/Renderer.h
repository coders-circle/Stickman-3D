#pragma once
#include "matrix.h"
#include "Rasterizer.h"
#include "Timer.h"

class Renderer
{
public:
    Renderer() : m_timer(200.0) {}
    void Initialize(const char* title, int x, int y, int width, int height);
    void MainLoop();
    void CleanUp();

    void Update(double deltaTime) {}
    
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
    void SetClearColor(RGBColor clearColor) { m_clearColor = clearColor; }

    template<int N>
    void DrawTriangle(Point<N>* pt1, Point<N>* pt2, Point<N>* pt3, void (*f)(Point<N>&))
    {
        Rasterizer::DrawTriangle(pt1, pt2, pt3, f, m_width, m_height); 
    }

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
    RGBColor m_clearColor;
};
