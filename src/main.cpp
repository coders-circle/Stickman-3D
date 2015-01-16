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

struct Vertex
{
    vec3 v;
    vec3 c;
};

mat4 transform, persp;
vec4 VertexShader(vec4 varying[], const Vertex& vertex)
{
    vec4 p = transform * vec4(vertex.v);
    varying[0] = vertex.c;
    return p;
}

// Vertex Buffer
Vertex vertices[] = 
{
    { vec3(-0.5f,  0.5f,  0.5f), vec3(1, 0, 0) },
    { vec3( 0.5f,  0.5f,  0.5f), vec3(0, 1, 0) },
    { vec3(-0.5f, -0.5f,  0.5f), vec3(0, 0, 1) },
    { vec3( 0.5f, -0.5f,  0.5f), vec3(1, 1, 0) },
    { vec3(-0.5f,  0.5f, -0.5f), vec3(1, 0, 1) },
    { vec3( 0.5f,  0.5f, -0.5f), vec3(0, 1, 1) },
    { vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, 1, 0.5f) },
    { vec3( 0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, 0.2f) },
};

// Index Buffer
uint16_t indices[] = 
{
    0, 1, 3, 0, 3, 2,
    4, 0, 2, 4, 2, 6,
    5, 4, 6, 5, 6, 7,
    1, 5, 7, 1, 7, 3,
    0, 4, 5, 0, 5, 1,
    3, 7, 6, 3, 6, 2,8
};


float angle=-45.0f*3.1415/180.0f;
void Render()
{
    transform = persp * Translate(vec3(0,0, -3.0f)) * RotateY(angle);
    g_renderer.DrawTriangles(&VertexShader, &FragmentShader, vertices, 8, indices, 12);
}

void Resize(int width, int height)
{
    persp = Perspective(60*3.1415f/180.0f, float(g_renderer.GetWidth())/float(g_renderer.GetHeight()), 0.1f, 100.0f);
}

void Update(double dt)
{
    angle += dt;
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
