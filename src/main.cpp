#include <common.h>
#include <Renderer.h>

Renderer g_renderer;
typedef Point<1> ClrPoint;          // Point<N> for point with N-varying attributes
                                    // A varying attribute is interpolated for intermediate pixels

void FragmentShader(ClrPoint& point)
{
    g_renderer.PutPixel(point.pos[0], point.pos[1], vec3(point.varying[0]));
}

vec4 VertexShader(vec4 varying[], const vec3& v, const vec3& c)
{
    varying[0] = c;
    return v;
}

vec3 vs[] = { vec3(0, 0.5f, 0), vec3(-0.5f, -0.5f, 0), vec3(0.5f, -0.5f, 0) };
vec3 cs[] = { vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f) };

void Render()
{
    //for (int i=0; i<500; ++i)
    // The final parameters are each an array of 3 elements,
    // Each element of each parameter is sent to vertex shader
    g_renderer.DrawTriangle(&VertexShader, &FragmentShader, vs, cs);
}

int main()
{
    g_renderer.Initialize("Test Renderer", 100, 100, 800, 600);
    g_renderer.SetClearColor(RGBColor(100, 149, 237));
    g_renderer.SetRenderCallback(&Render);
    
    g_renderer.MainLoop();

    g_renderer.CleanUp();
        return 0;
}
