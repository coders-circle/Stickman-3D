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

// Bias matrix for texture transformation
mat4 bias_matrix
(
    0.5f, 0, 0, 0.5f,
    0, -0.5f, 0, 0.5f,
    0, 0, 0.5f, 0.5f,
    0, 0, 0, 1
);

float angle=(180)*3.1415f/180.0f;
// Render objects
void Render()
{
    // Some animations
    auto trans = g_entities[0].GetComponent<TransformComponent>();
    //trans->SetRotation(vec3(-90*3.1415f/180.0f, angle, 0));
    //trans = g_entities[2].GetComponent<TransformComponent>();
    //trans->SetRotation(vec3(0, -angle, 0));

    trans = g_entities[3].GetComponent<TransformComponent>();
    trans->SetTransform(LookAt(vec3(cosf(angle)*4, 2, sinf(angle)*4), vec3(0,0,0), vec3(0,1,0)).AffineInverse());

    // First Pass:
    // Create depth buffer in light space
    g_renderer.UseDepthBuffer(1);
    g_renderer.ClearDepth();
    for (size_t i=0; i<g_systems.size(); ++i)
        g_systems[i]->RenderShadow();

    // Second Pass:
    // Render the scene and use previous depth buffer for shadow mapping
    g_renderer.UseDepthBuffer(0);
    g_renderer.ClearColorAndDepth();
    for (size_t i=0; i<g_systems.size(); ++i)
        g_systems[i]->Render();

    // Third Pass:
    // Render the scene with transparent objects
    for (size_t i=0; i<g_systems.size(); ++i)
        g_systems[i]->PostRender();
}

// On resize of window, we calculate the projection matrix
void Resize(int width, int height)
{
    mat4 proj = Orthographic(-5, 5, -5, 5, -10.0f, 10.0f);
    mat4 view = LookAt(-g_renderer.light.direction, vec3(0,0,0), vec3(0,1,0));
    g_renderer.transforms.light_vp = proj*view;

    for (size_t i=0; i<g_systems.size(); ++i)
        g_systems[i]->Resize(width, height);
}

double animtime;
Mesh* g_stickmesh = NULL;
// Update each frame by time-step dt
void Update(double dt)
{
    angle += float(dt/10);
    for (size_t i=0; i<g_systems.size(); ++i)
        g_systems[i]->Update(dt);

    if (g_stickmesh)
    {
        animtime += dt/4;
        if (animtime > g_stickmesh->GetAnimation()->duration)
            animtime -= g_stickmesh->GetAnimation()->duration;
        g_stickmesh->Animate(animtime);
    }
}

#ifdef _WIN32
#pragma comment(lib, "SDL2.lib")
#define SDL_main main
#endif

int main()
{
    g_renderer.Initialize("Stickman-3D", 100, 100, 800, 600);
    g_renderer.SetClearColor(RGBColor(100, 149, 237));
    g_renderer.SetRenderCallback(&Render);
    g_renderer.SetUpdateCallback(&Update);
    g_renderer.SetResizeCallback(&Resize);
    
    // Add depth buffer for shadow mapping
    g_renderer.AddDepthBuffer();

    // Light Direction for a directional light
    g_renderer.light.direction = vec3(-1.5, -2, -1);
    g_renderer.light.direction.Normalize();
    // Light intenisities
    const float ambient = 0.2f;
    g_renderer.light.ambient = vec3(ambient, ambient, ambient);
    g_renderer.light.diffuse = vec3(1.0f, 1.0f, 1.0f);
    g_renderer.light.specular = vec3(1.0f, 1.0f, 1.0f);

    // Add systems
    CameraSystem cameraSystem(&g_renderer);
    g_systems.push_back(&cameraSystem);
    MeshRenderSystem<DiffuseMaterial> diffuseRenderSystem(&g_renderer);
    MeshRenderSystem<SpecularMaterial> specularRenderSystem(&g_renderer);
    g_systems.push_back(&diffuseRenderSystem);
    g_systems.push_back(&specularRenderSystem);

    // Create some entities
    g_entities.resize(5);
    
    // MeshComponent<MaterialType> is a component to store a mesh and a material
    typedef MeshComponent<DiffuseMaterial> DiffuseMeshComponent;  // DiffuseMaterial supports diffuse color, texture and shadow on surface
    typedef MeshComponent<SpecularMaterial> SpecularMeshComponent; // SpecularMaterial also supports a specular color and shininess
    
    // Stickman entity, with mesh loaded from file
    auto msc = g_entities[0].AddComponent<SpecularMeshComponent>(0.25f);
    g_stickmesh = &msc->mesh;
    msc->mesh.LoadAnimatedFile("test1.dat");
    msc->material.depthBias = 0.05f;
    msc->material.shininess = 7.0f;
    msc->material.specularColor = vec3(1.0f, 1.0f, 1.0f);
    g_entities[0].AddComponent<TransformComponent>(vec3(0,0.45f,0), vec3(-90*3.1415f/180.0f,0,0));
    
    // Ground entity, with box mesh and green diffuse color
    auto mc = g_entities[1].AddComponent<DiffuseMeshComponent>();
    mc->mesh.LoadBox(3.0f, 0.05f, 3.0f);        // Larger than this ground size seems to give problems while shadow mapping; so use smaller pieces of ground entities instead of one large box
    mc->material.depthBias = 0.0f;
    mc->material.diffuseColor = vec3(0.0f, 1.0f, 0.0f);
    g_entities[1].AddComponent<TransformComponent>(vec3(0,-1.05f,0));
    
    // Cube entity, with box mesh and texture loaded from file
    mc = g_entities[2].AddComponent<DiffuseMeshComponent>(1.0f);
    mc->mesh.LoadBox(0.5f, 0.5f, 0.5f);
    //mc->mesh.LoadSphere(0.7f, 40, 40);
    mc->material.depthBias = 0.008f;
    mc->material.textureId = g_textureManager.AddTexture("grass_T.bmp");
    g_entities[2].AddComponent<TransformComponent>(vec3(2,-0.5f,-1));

    // A camera entity
    g_entities[3].AddComponent<CameraComponent>();
    auto t = g_entities[3].AddComponent<TransformComponent>();
    t->SetTransform(LookAt(vec3(-3, 2.0f, -3.0f), vec3(0,1,0), vec3(0,1,0)).AffineInverse());
 
    // Test transparent entity
    mc = g_entities[4].AddComponent<DiffuseMeshComponent>();
    mc->mesh.LoadSphere(0.5f, 30, 30);
    //mc->mesh.LoadBox(0.5f, 0.5f, 0.5f);
    mc->material.depthBias = 0.0f;
    mc->material.diffuseColor = vec4(1, 0, 0, 0.4f);
    mc->transparent = true;
    g_entities[4].AddComponent<TransformComponent>(vec3(-1.05f, 0, 0));


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
