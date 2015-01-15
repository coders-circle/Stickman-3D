#include <common.h>
#include <Renderer.h>
#include <transform.h>

Renderer g_renderer;
typedef Point<1> ClrPoint;          // Point<N> for point with N-varying attributes
                                    // A varying attribute is interpolated for intermediate pixels

void FragmentShader(ClrPoint& point)
{
    g_renderer.PutPixel(point.pos[0], point.pos[1], vec3(point.varying[0]));
}

float t=0.0f;
mat4 trans, persp;
vec4 VertexShader(vec4 varying[], const vec3& v, const vec3& c)
{
    vec4 p = trans * vec4(v);
    varying[0] = c;
    return p;
}

vec3 vs[] = { vec3(0, 0.5f, 0), vec3(-0.5f, -0.5f, 0), vec3(0.5f, -0.5f, 0) };
vec3 cs[] = { vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f) };

void Render()
{
    trans = persp * Translate(vec3(0,0, -1.0f)) * RotateY(t);
    g_renderer.DrawTriangle(&VertexShader, &FragmentShader, vs, cs);
}

void Resize(int width, int height)
{
    persp = Perspective(60*3.1415f/180.0f, float(g_renderer.GetWidth())/float(g_renderer.GetHeight()), 0.1f, 100.0f);
}

void Update(double dt)
{
    t += dt;
}

int main()
{
    g_renderer.Initialize("Test Renderer", 100, 100, 800, 600);
    g_renderer.SetClearColor(RGBColor(100, 149, 237));
    g_renderer.SetRenderCallback(&Render);
    g_renderer.SetUpdateCallback(&Update);
    g_renderer.SetResizeCallback(&Resize);

    Resize(g_renderer.GetWidth(), g_renderer.GetHeight());
    
    g_renderer.MainLoop();

    g_renderer.CleanUp();
        return 0;
}
