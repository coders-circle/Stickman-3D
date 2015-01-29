#include <common.h>
#include <Renderer.h>
#include <transform.h>
#include <TextureManager.h>
#include <Mesh.h>

// Shaders to generate depth buffer in light space
#include <shaders/depth_shaders3d.h>

#include <System.h>
#include <materials.h>

Renderer g_renderer;                // Default renderer
TextureManager g_textureManager;    // Default texture manager

/*
    Entity-Component-System approach of programming
    ==============================================
    Entity : Anything from game character and objects to lights, cameras and sounds.
             Any entity is just a collection of components.
    Component : A set of data or attributes.
                Like MeshComponent (mesh and material data), TransformComponent(position and rotation data).
                A collection of components make up an entity.
    System : A processor that works on certain entities, containing particular set of components.
             A MeshRenderSystem, for instance, draw all entities with Mesh and Transform components using renderer.
*/
std::vector<Entity> g_entities;     // Collection of all entities
std::vector<SystemBase*> g_systems; // Collection of all systems

// View-Projection matrices for camera space and light space
mat4 vp, light_vp;

// Bias matrix for texture transformation
mat4 bias_matrix
(
    0.5f, 0, 0, 0.5f,
    0, -0.5f, 0, 0.5f,
    0, 0, 0.5f, 0.5f,
    0, 0, 0, 1
);

float angle=45.0f*3.1415f/180.0f;
// Render objects
void Render()
{
    auto trans = g_entities[0].GetComponent<TransformComponent>();
    trans->SetRotation(vec3(-90*3.1415f/180.0f, angle, 0));
    trans = g_entities[2].GetComponent<TransformComponent>();
    trans->SetRotation(vec3(0, -angle, 0));
    
    for (size_t i=0; i<g_systems.size(); ++i)
        g_systems[i]->RenderShadow();
    for (size_t i=0; i<g_systems.size(); ++i)
        g_systems[i]->Render();
}

// On resize of window, we calculate the projection matrix
void Resize(int width, int height)
{
    mat4 proj = Orthographic(-5, 5, -5, 5, -10.0f, 10.0f);
    mat4 view = LookAt(-g_renderer.lights.lightDirection, vec3(0,0,0), vec3(0,1,0));
    light_vp = proj*view;

    view = LookAt(vec3(-3, 0.1f, -3.0f), vec3(0,0,0), vec3(0,1,0));
    proj = Perspective(60*3.1415f/180.0f, float(width)/float(height), 0.1f, 100.0f);
    vp = proj*view;

    for (size_t i=0; i<g_systems.size(); ++i)
        g_systems[i]->Resize(width, height);
}

// Update each frame by time-step dt
void Update(double dt)
{
    angle += float(dt);
    for (size_t i=0; i<g_systems.size(); ++i)
        g_systems[i]->Update(dt);
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
    
    // Add depth buffer for shadow mapping
    g_renderer.AddDepthBuffer();

    // Light Direction for a directional light
    g_renderer.lights.lightDirection = vec3(-1.5, -2, -1);
    g_renderer.lights.lightDirection.Normalize();

    // Add systems
    MeshRenderSystem<TextureShadowMaterial> diffuseRenderSystem(&g_renderer);
    g_systems.push_back(&diffuseRenderSystem);

    // Create some entities
    g_entities.resize(3);
    
    // MeshComponent<MaterialType> is a component to store a mesh and a material
    typedef MeshComponent<TextureShadowMaterial> DiffuseMeshComponent;  // TextureShadowMaterial supports texture and shadow on surface
    
    // Stickman entity, with mesh loaded from file
    auto mc = g_entities[0].AddComponent<DiffuseMeshComponent>(0.25f);
    mc->mesh.LoadFile("test.dat");
    mc->material.depthBias = 0.05f;
    g_entities[0].AddComponent<TransformComponent>(vec3(0,-1,0), vec3(-90*3.1415f/180.0f,0,0));
    
    // Ground entity, with box mesh and green diffuse color
    mc = g_entities[1].AddComponent<DiffuseMeshComponent>();
    mc->mesh.LoadBox(3.0f, 0.05f, 3.0f);
    mc->material.depthBias = 0.0f;
    mc->material.diffuseColor = vec3(0.0f, 1.0f, 0.0f);
    g_entities[1].AddComponent<TransformComponent>(vec3(0,-1.05f,0));
    
    // Cube entity, with box mesh and texture loaded from file
    mc = g_entities[2].AddComponent<DiffuseMeshComponent>(0.5f);
    mc->mesh.LoadBox(0.5f, 0.5f, 0.5f);
    mc->material.depthBias = 0.008f;
    mc->material.textureId = g_textureManager.AddTexture("grass_T.bmp");
    g_entities[2].AddComponent<TransformComponent>(vec3(2,-0.5f,-1));
    
    // Add entities to all systems and initialize the systems
    for (size_t j=0; j<g_systems.size(); ++j)
    {
        for (size_t i=0; i<g_entities.size(); ++i)
            g_systems[j]->AddEntity(&g_entities[i]);
        g_systems[j]->Initialize();
    }

    // Call resize once to initialize the view and projection matrices
    Resize(g_renderer.GetWidth(), g_renderer.GetHeight());
    
    g_renderer.MainLoop();

    // Clean up the systems
    for (size_t i=0; i<g_systems.size(); ++i)
        g_systems[i]->CleanUp();

    g_renderer.CleanUp();
        return 0;
}
