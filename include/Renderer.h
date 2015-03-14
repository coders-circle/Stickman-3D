#pragma once
#include "quat.h"
#include "Timer.h"
#include "Rasterizer.h"
#include <RenderThreadManager.h>

//#define USE_MULTITHREADING

// Renderer responsible for managing the window
//  and drawing pixels and triangles
class Renderer
{
public:
    Renderer();
    ~Renderer();

    void Initialize(const char* title, int x, int y, int width, int height);
    void MainLoop();
    void CleanUp();
    
    void PutPixel(int x, int y, const RGBColor &color)
    {
        if (y > m_height || x > m_width)
            return;
        PutPixelUnsafe(x, y, color);
    }
    // (Maybe) faster due to no validity check
    void PutPixelUnsafe(int x, int y, const RGBColor &color)
    {
        m_framebuffer[y*m_width + x] = (0xFF << 24) | (color.r << 16) | (color.g << 8) | color.b;
    }

    // Alpha/Transparency capabilities
    void PutPixel(int x, int y, const RGBColor &color, float alpha)
    {
        if (y > m_height || x > m_width)
            return;
        PutPixelUnsafe(x, y, color, alpha);
    }
    void PutPixelUnsafe(int x, int y, RGBColor color, float alpha)
    {
        if (alpha == 0.0f)
            return;
        if (alpha < 1.0f)
        {
            RGBColor d = GetPixel(x, y);
            color.r = uint8_t(d.r * alpha + color.r * (1.0f-alpha));
            color.g = uint8_t(d.g * alpha + color.g * (1.0f-alpha));
            color.b = uint8_t(d.b * alpha + color.b * (1.0f-alpha));
        }
        m_framebuffer[y*m_width + x] = (0xFF << 24) | (color.r << 16) | (color.g << 8) | color.b;
    }
    
    RGBColor GetPixel(int x, int y)
    {
        uint32_t color = m_framebuffer[y*m_width + x];
        return RGBColor((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    }

    void SetRenderCallback(std::function<void()> renderCallback) { m_render = renderCallback; }
    void SetUpdateCallback(std::function<void(double)> updateCallback) { m_update = updateCallback; }
    void SetResizeCallback(std::function<void(int, int)> resizeCallback) { m_resize = resizeCallback; }

    // Draw a triangle from from pixel points
    template<int N>
    void DrawTriangle(Point<N> &pt1, Point<N> &pt2, Point<N> &pt3, void (*fragmentShader)(Point<N>&), bool transparency = false)
    {
        Rasterizer::DrawTriangle(&pt1, &pt2, &pt3, fragmentShader, m_width, m_height, m_depthBuffers[m_depthBufferId], transparency); 
    }
    
    // Draw triangles with given vertices and indices
    //  The vertices are passed through the vertexShader function
    //  and rasterized. Each pixel is then passed through the framentShader function
    template<int N, class Args>
    void DrawTriangles(vec4(*vertexShader)(vec4[], const Args&), void(*fragmentShader)(Point<N>&), Args* vertexBuffer, size_t numVertices, uint16_t* indexBuffer, size_t numTriangles, bool backfaceVisible = false, bool transparency = false)
    {
        vec4* vs = new vec4[numVertices];                // array to carry the clip-space vertices returned by vertexBuffer
        Point<N>* points = new Point<N>[numVertices];    // array to carry window space points and their attributes

        ProcessVertices(points, vs, vertexShader, vertexBuffer, numVertices);
#ifndef USE_MULTITHREADING
        m_threader.DrawTriangles(fragmentShader, indexBuffer, numTriangles, backfaceVisible, vs, points, transparency);
#else
        m_threader.DrawTrianglesThreaded(fragmentShader, indexBuffer, numTriangles, backfaceVisible, vs, points, transparency);
#endif
        delete[] points;
        delete[] vs;
    }
        
    // Process each vertex through vertexShader and
    //  fill 'newVertices' with resulting clip-space vertices
    //  and 'points' with corresponding window-space points and their attributes
    template<int N, class Args>
    void ProcessVertices(Point<N>*points, vec4* newVertices, vec4(*f)(vec4[], const Args&), Args* args, size_t numVertices)
    {
        vec4 v;
        for (size_t i=0; i<numVertices; ++i)
        {
            newVertices[i] = f(points[i].attribute, args[i]);
            bool t = false;
            for (int j=0; j<N; ++j)
                points[i].attribute[j] = points[i].attribute[j];
            if (newVertices[i].w == 0.0f)
                newVertices[i].w = 0.000001f; // avoid divide by zero
           
            ///#### TEMPORARY SOLUTION
            else if (newVertices[i].w < 0)
            {
                t = true;
                newVertices[i].w *= -1;       // Seems to solve the problem when object is partially behind eye and partially in front
                                              // Hasn't been tested for all cases
            }
            ///###

            v = vec4(newVertices[i].ConvertToVec3(), newVertices[i].w);
            v.x = (0.5f*v.x + 0.5f)*m_width;
            v.y = (-0.5f*v.y + 0.5f)*m_height;

            ///### TEMPORARY SOLUTION
            if (t) v.z += 2.0f;
            ///###

            v.z = (0.5f*v.z + 0.5f);
            points[i].FromVec4(v);
        }
    }
    
    int GetWidth() { return m_width; }
    int GetHeight() { return m_height; }
    void SetClearColor(RGBColor clearColor) { m_clearColor = clearColor; }

    size_t AddDepthBuffer() { m_depthBuffers.push_back(new float[m_width*m_height]); return m_depthBuffers.size()-1; }
    void UseDepthBuffer(size_t depthBufferId) { m_depthBufferId = depthBufferId; }
    float* GetDepthBuffer(size_t depthBufferId) { return m_depthBuffers[depthBufferId]; }

    // Clear the color-buffer
    void ClearColor()
    {
        for (int i = 0; i < m_width; ++i)
        for (int j = 0; j < m_height; ++j)
            PutPixelUnsafe(i, j, m_clearColor); // Clear the color buffer
    }
    void ClearColorAndDepth()
    {
        for (int i = 0; i < m_width; ++i)
        for (int j = 0; j < m_height; ++j)
        {
            PutPixelUnsafe(i, j, m_clearColor); // Clear the color buffer
            m_depthBuffers[m_depthBufferId][j*m_width+i] = 1.0f; // Clear the depth buffer
        }
    }
    void ClearDepth()
    {
        for (int i = 0; i < m_width; ++i)
        for (int j = 0; j < m_height; ++j)
            m_depthBuffers[m_depthBufferId][j*m_width+i] = 1.0f; // Clear the depth buffer
    }

    struct
    {
        mat4 
            mvp,            // Model-View-Projection composite matrix
            model,          // Model matrix
            bias_light_mvp, // Texture matrix == Model-View-Projection matrix for light space combined with bias matrix

            vp,             // View-Projection matrix
            light_vp;       // View-Projection matrix for light space

        vec3 camPos;        // Position of camera needed for some lighting calculations

    } transforms;
    
    struct
    {
        vec3 direction;
        vec3 diffuse, specular, ambient;
    } light;

private:
    uint32_t* m_framebuffer;
    int m_width, m_height;
    Timer m_timer;
    
    SDL_Window* m_window;
    SDL_Surface* m_screen;
    std::vector<float*> m_depthBuffers;
    size_t m_depthBufferId;

    std::function<void()> m_render;
    std::function<void(double)> m_update;
    std::function<void(int, int)> m_resize;
    RGBColor m_clearColor;

    RenderThreadManager m_threader;
};

// A class to store shaders
// Shaders are stored as template arguments, which
// MIGHT help compile time optimization
template<Renderer& renderer, class VertexType, int NoOfAttributes,
        vec4(*vertexShader)(vec4[], const VertexType&), void(*fragmentShader)(Point<NoOfAttributes>&), bool backfaceVisible=false>
class Shaders
{
public:
    void DrawTriangles(std::vector<VertexType>& vertices, std::vector<uint16_t>& indices, bool transparency=false)
    {
        renderer.DrawTriangles(vertexShader, fragmentShader, &vertices[0], vertices.size(), &indices[0], indices.size()/3, backfaceVisible, transparency);
    }
};

template<int N>
inline void RenderThreadManager::DrawTriangles(void(*fragmentShader)(Point<N>&), uint16_t* indexBuffer, size_t numTriangles, bool backfaceVisible,
                    vec4* vs, Point<N>* points, bool transparency, int offset)
{
    indexBuffer += offset*3;
    for (size_t i=0; i<numTriangles; ++i)
    {
        size_t i1 = indexBuffer[i*3], i2 = indexBuffer[i*3+1], i3 = indexBuffer[i*3+2];

        // Clip-Space clipping
        if ((vs[i1].x < -vs[i1].w && vs[i2].x < -vs[i2].w && vs[i3].x < -vs[i3].w) ||
            (vs[i1].y < -vs[i1].w && vs[i2].y < -vs[i2].w && vs[i3].y < -vs[i3].w) ||
            (vs[i1].z < -vs[i1].w && vs[i2].z < -vs[i2].w && vs[i3].z < -vs[i3].w) ||
            (vs[i1].x > vs[i1].w && vs[i2].x > vs[i2].w && vs[i3].x > vs[i3].w) ||
            (vs[i1].y > vs[i1].w && vs[i2].y > vs[i2].w && vs[i3].y > vs[i3].w) ||
            (vs[i1].z > vs[i1].w && vs[i2].z > vs[i2].w && vs[i3].z > vs[i3].w))
            continue;

        // BackFace or FrontFace Culling
        int C = (points[i2].x-points[i1].x) * (points[i3].y-points[i1].y)
                - (points[i3].x-points[i1].x) * (points[i2].y-points[i1].y);
        if (backfaceVisible?C > 0:C < 0)
           renderer->DrawTriangle(points[i1], points[i2], points[i3], fragmentShader, transparency);
    }
}
