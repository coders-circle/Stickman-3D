#include <common.h>
#include <Renderer.h>
#include <transform.h>

Renderer g_renderer;
typedef Point<1> ClrPoint;          // Point<N> for point with N-varying attributes
                                    // A varying attribute is interpolated for intermediate pixels

void FragmentShader(ClrPoint& point)
{
    vec3 n = point.varying[0];
    vec3 c(1.0f, 1.0f, 1.0f);
    n.Normalize();
    vec3 dir(-1,0,0);
    float intensity = Max(n.x*dir.x + n.y*dir.y + n.z*dir.z, 0.0f);
    c = c*intensity;
    g_renderer.PutPixel(point.pos[0], point.pos[1], c);
}

struct Vertex
{
    vec3 v;
    vec3 n;
};

mat4 transform, model, persp;
vec4 VertexShader(vec4 varying[], const Vertex& vertex)
{
    vec4 p = transform * vec4(vertex.v);
    varying[0] = model * vertex.n;
    return p;
}

// Vertex Buffer
Vertex vertices[] = 
{
    // FRONT
    { vec3(-0.5f,  0.5f,  0.5f), vec3(0, 0, 1) },
    { vec3( 0.5f,  0.5f,  0.5f), vec3(0, 0, 1) },
    { vec3(-0.5f, -0.5f,  0.5f), vec3(0, 0, 1) },
    { vec3( 0.5f, -0.5f,  0.5f), vec3(0, 0, 1) },
    // RIGHT
    { vec3( 0.5f,  0.5f, -0.5f), vec3(1, 0, 0) },
    { vec3( 0.5f,  0.5f,  0.5f), vec3(1, 0, 0) },
    { vec3( 0.5f, -0.5f, -0.5f), vec3(1, 0, 0) },
    { vec3( 0.5f, -0.5f,  0.5f), vec3(1, 0, 0) },
    // LEFT
    { vec3(-0.5f,  0.5f,  0.5f), vec3(-1, 0, 0) },
    { vec3(-0.5f,  0.5f, -0.5f), vec3(-1, 0, 0) },
    { vec3(-0.5f, -0.5f,  0.5f), vec3(-1, 0, 0) },
    { vec3(-0.5f, -0.5f, -0.5f), vec3(-1, 0, 0) },
    // TOP
    { vec3(-0.5f,  0.5f, -0.5f), vec3(0, 1, 0) },
    { vec3( 0.5f,  0.5f, -0.5f), vec3(0, 1, 0) },
    { vec3(-0.5f,  0.5f,  0.5f), vec3(0, 1, 0) },
    { vec3( 0.5f,  0.5f,  0.5f), vec3(0, 1, 0) },
    // BOTTOM
    { vec3(-0.5f, -0.5f,  0.5f), vec3(0, -1, 0) },
    { vec3( 0.5f, -0.5f,  0.5f), vec3(0, -1, 0) },
    { vec3(-0.5f, -0.5f, -0.5f), vec3(0, -1, 0) },
    { vec3( 0.5f, -0.5f, -0.5f), vec3(0, -1, 0) },
    // BACK
    { vec3( 0.5f,  0.5f, -0.5f), vec3(0, 0, -1) },
    { vec3(-0.5f,  0.5f, -0.5f), vec3(0, 0, -1) },
    { vec3( 0.5f, -0.5f, -0.5f), vec3(0, 0, -1) },
    { vec3(-0.5f, -0.5f, -0.5f), vec3(0, 0, -1) },
};

// Index Buffer
uint16_t indices[] = 
{
    0, 1, 3, 0, 3, 2,
    4, 5, 7, 4, 7, 6,
    8, 9, 11, 8, 11, 10,
    12, 13, 15, 12, 15, 14,
    16, 17, 19, 16, 19, 18,
    20, 21, 23, 20, 23, 22
};


float angle=-45.0f*3.1415/180.0f;
void Render()
{
    model = Translate(vec3(0,0,-3))*RotateY(angle);
    transform = persp * model;
    g_renderer.DrawTriangles(&VertexShader, &FragmentShader, vertices, 24, indices, 12);
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
