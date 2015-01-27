#include <common.h>
#include <Renderer.h>
#include <transform.h>
#include <TextureManager.h>
#include <Mesh.h>
#include <materials.h>

Renderer g_renderer;
TextureManager g_textureManager;

Mesh<TextureShadowMaterial> g_meshes[3];

// View-Projection matrices for camera space and light space
mat4 vp, light_vp;

// Bias matrix to map
//  from (-1,1) range to (0,1) ranges
//  required for depth buffer sampling
// Since this can be combined with mvp matrix,
//  the mapping is automatically done along with
//  mvp transformation
mat4 bias_matrix
(
    0.5f, 0, 0, 0.5f,
    0, -0.5f, 0, 0.5f,
    0, 0, 0.5f, 0.5f,
    0, 0, 0, 1
);

// Shaders to generate depth buffer in light space
#include <shaders/depth_shaders3d.h>

float angle=45.0f*3.1415f/180.0f;
mat4 Rotate = RotateX(-90*3.1415f/180.0f);
// Render objects
void Render()
{
    mat4 models[] = { 
        Translate(vec3(0,-1.0f,0))*RotateY(angle)*Rotate*Scale(0.25f),
        Translate(vec3(0, -1.05f, 0)),
        Translate(vec3(-1.0f,-0.0f,-0.3f))*RotateY(-angle)*Scale(0.5f),
    };

    // First Pass:
    // Create depth buffer in light space
    g_renderer.UseDepthBuffer(1);
    g_renderer.ClearDepth();

    for (auto i=0; i<3; ++i)
    {
        g_renderer.transforms.model = models[i];
        g_renderer.transforms.mvp = light_vp * g_renderer.transforms.model;
        g_meshes[i].Draw(shadersDepth);
    }

    // Second Pass:
    // Render the scene and use previous depth buffer for shadow mapping
    g_renderer.UseDepthBuffer(0);
    g_renderer.ClearColorAndDepth();

    for (auto i=0; i<3; ++i)
    {
        g_renderer.transforms.model = models[i];
        g_renderer.transforms.mvp = vp * g_renderer.transforms.model;
        g_renderer.transforms.bias_light_mvp = bias_matrix * light_vp * g_renderer.transforms.model;
        g_meshes[i].Draw();
    }
}

// On resize of window, we calculate the projection matrix
void Resize(int width, int height)
{   
    mat4 proj = Orthographic(-5, 5, -5, 5, -10.0f, 10.0f);
    mat4 view = LookAt(-g_renderer.lights.lightDirection, vec3(0,0,0), vec3(0,1,0));
    light_vp = proj*view;

    view = LookAt(vec3(3, 1, 0), vec3(0,0,0), vec3(0,1,0));
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
    
    // Add depth buffer for shadow mapping
    g_renderer.AddDepthBuffer();

    // Light Direction for a directional light
    g_renderer.lights.lightDirection = vec3(-1.5, -2, -1);
    g_renderer.lights.lightDirection.Normalize();

    // Create some meshes
    g_meshes[0].LoadFile("test.dat");
    g_meshes[1].LoadBox(3.0f, 0.05f, 3.0f);
    g_meshes[2].LoadBox(0.5f, 0.5f, 0.5f);

    // Load texture
    g_meshes[2].material.textureId = g_textureManager.AddTexture("grass_T.bmp");
    
    // Correct depth bias are required for correct shadow on meshes' surfaces
    g_meshes[0].material.depthBias = 0.05f;
    g_meshes[1].material.depthBias = 0.0f;//-0.000005f;
    g_meshes[2].material.depthBias = 0.008f;
    
    
    // Call resize once to initialize the view and projection matrices
    Resize(g_renderer.GetWidth(), g_renderer.GetHeight());
    
    g_renderer.MainLoop();

    g_renderer.CleanUp();
        return 0;
}
