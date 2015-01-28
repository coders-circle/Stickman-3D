#include <common.h>
#include <Renderer.h>
#include <transform.h>
#include <TextureManager.h>
#include <Mesh.h>

Renderer g_renderer;
TextureManager g_textureManager;
Mesh g_mesh;


mat4 transform, model, persp;
size_t texId;

// VertexShader is called for each vertex and is expected to return its
//  position in clip space as well as its attributes
vec4 VertexShader(vec4 attribute[], const Vertex& vertex)
{
    vec4 p = transform * vec4(vertex.position);
    attribute[0] = mat3(model) * vertex.normal;
    attribute[1] = vertex.texcoords;
    return p;
}

// This function is called for each pixel
// The Point contains x,y position of the pixel,
//  the depth value and the interpolated attributes
void FragmentShader(Point<2>& point)
{
    vec3 n = point.attribute[0];
    n.Normalize();
    vec3 c = g_textureManager.GetTexture(texId).Sample(point.attribute[1]);
        
    // Perform a simple phong based lighting calculation for directional light
    vec3 dir(-1,0,-1);                   
    dir.Normalize();
    float intensity = Min(Max(n.dot(-dir), 0.0f) + 0.09f, 1.0f);
    c = c*intensity;    // "Light" the color
    
    g_renderer.PutPixel(point.pos[0], point.pos[1], c);     // Use the calculated color to plot the pixel
}

auto shaders = 
//              Shaders<Renderer&, VertexClass, NumberOfAttributes, VertexShaderFunction, FragmentShaderFunction>
                Shaders<g_renderer, Vertex, 2, &VertexShader, &FragmentShader>();

float angle=45.0f*3.1415f/180.0f;
mat4 Rotate = RotateX(-90*3.1415f/180.0f);
// Render objects
void Render()
{
    model = Translate(vec3(0,-1,-3))*RotateY(angle)*Rotate*Scale(0.25f);
    transform = persp * model;  

    texId = g_mesh.GetTextureId();
    g_mesh.Draw(shaders);
}

// On resize of window, we calculate the projection matrix
void Resize(int width, int height)
{
    persp = Perspective(60*3.1415f/180.0f, float(g_renderer.GetWidth())/float(g_renderer.GetHeight()), 0.1f, 100.0f);
}

// Update each frame by time-step dt
void Update(double dt)
{
    angle += dt;
}

#ifdef _WIN32
#pragma comment(lib, "SDL2.lib")
#define SDL_main main
#endif

int main()
{
    g_renderer.Initialize("Test Renderer", 100, 100, 800, 600);
    g_renderer.SetClearColor(RGBColor(100, 149, 237));
    g_renderer.SetRenderCallback(&Render);
    g_renderer.SetUpdateCallback(&Update);
    g_renderer.SetResizeCallback(&Resize);
    
    // And create the mesh
    //g_mesh.LoadBox(0.5f, 0.5f, 0.5f);
    //g_mesh.LoadSphere(0.5f, 42, 42);
    g_mesh.LoadFile("test.dat");
    // Load the texture
//    g_mesh.SetTextureId(g_textureManager.AddTexture("grass_T.bmp"));
    
    // Call resize once to initialize the projection matrix
    Resize(g_renderer.GetWidth(), g_renderer.GetHeight());
    
    g_renderer.MainLoop();

    g_renderer.CleanUp();
        return 0;
}
