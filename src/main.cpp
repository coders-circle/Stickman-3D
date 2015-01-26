#include <common.h>
#include <Renderer.h>
#include <transform.h>
#include <TextureManager.h>
#include <Mesh.h>

Renderer g_renderer;
TextureManager g_textureManager;
Mesh g_mesh;
Mesh g_mesh2;


mat4 transform, model;
mat4 vp, light_vp, bias_light_mvp ;
size_t texId;
vec3 lightDir(-1.5, -2, -1);

mat4 bias_matrix
(
    0.5f, 0, 0, 0.5f,
    0, -0.5f, 0, 0.5f,
    0, 0, 0.5f, 0.5f,
    0, 0, 0, 1
)
;

#include <shaders/shaders3d.h>
#include <shaders/depth_shaders3d.h>

float angle=45.0f*3.1415f/180.0f;
mat4 Rotate = RotateX(-90*3.1415f/180.0f);
// Render objects
void Render()
{
    g_renderer.UseDepthBuffer(1);
    g_renderer.ClearDepth();
    model = Translate(vec3(0,-1,0))*RotateY(angle)*Rotate*Scale(0.25f);
    transform = light_vp * model;  
    texId = g_mesh.GetTextureId();
    g_mesh.Draw(shadersDepth);

    model = Translate(vec3(0, -1-0.05f, 0));
    transform = light_vp * model;
    texId = g_mesh2.GetTextureId();
    g_mesh2.Draw(shadersDepth);

    g_renderer.UseDepthBuffer(0);
    g_renderer.ClearColorAndDepth();
    model = Translate(vec3(0,-1,0))*RotateY(angle)*Rotate*Scale(0.25f);
    transform = vp * model;
    bias_light_mvp = bias_matrix * light_vp * model;
    texId = g_mesh.GetTextureId();
    g_mesh.Draw(shaders);

    model = Translate(vec3(0, -1-0.05f, 0));
    transform = vp * model;
    bias_light_mvp = bias_matrix * light_vp * model;
    texId = g_mesh2.GetTextureId();
    g_mesh2.Draw(shaders);
}

// On resize of window, we calculate the projection matrix
void Resize(int width, int height)
{   
    mat4 proj = Orthographic(-3, 3, -3, 3, -10.0f, 10.0f);
    mat4 view = LookAt(-lightDir, vec3(0,0,0), vec3(0,1,0));
    light_vp = proj*view;

    view = LookAt(vec3(-3, 1, 4), vec3(0,0,0), vec3(0,1,0));
    proj = Perspective(60*3.1415f/180.0f, float(width)/float(height), 0.1f, 100.0f);
    vp = proj*view;
}

// Update each frame by time-step dt
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
    

    g_renderer.AddDepthBuffer();
    // And create the mesh
    //g_mesh.LoadBox(0.5f, 0.5f, 0.5f);
    //g_mesh.LoadSphere(5.0f, 42, 42);
    g_mesh.LoadFile("test.dat");
    g_mesh2.LoadBox(3.0f, 0.05f, 3.0f);
    // Load the texture
   // g_mesh.SetTextureId(g_textureManager.AddTexture("grass_T.bmp"));
    
    // Call resize once to initialize the projection matrix
    Resize(g_renderer.GetWidth(), g_renderer.GetHeight());
    
    g_renderer.MainLoop();

    g_renderer.CleanUp();
        return 0;
}
