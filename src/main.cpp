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
    trans->SetTransform(LookAt(vec3(cosf(angle)*5, 2, sinf(angle)*5), vec3(0,0,0), vec3(0,1,0)).AffineInverse());

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
bool firstTime = true;
// Update each frame by time-step dt
void Update(double dt)
{
    bool animating = false;
    if (firstTime) {
        animating = true;
        firstTime = false;
    }
    //angle += float(dt/10);
    const uint8_t* keys = SDL_GetKeyboardState(NULL);
    auto trans = g_entities[0].GetComponent<TransformComponent>();
    vec3 rot = trans->GetRotation();
    if (keys[SDL_SCANCODE_RIGHT])
        rot[1] -= (float)dt;
    if (keys[SDL_SCANCODE_LEFT])
        rot[1] += (float)dt;
    trans->SetRotation(rot);
    mat3 t = EulerXYZ(rot);
    vec3 pos = trans->GetPosition();

    vec3 dir(t[0][0], t[1][0], t[2][0]);
    /*if (keys[SDL_SCANCODE_A])
        pos = pos + dir*(float)dt*0.5f; 
    if (keys[SDL_SCANCODE_D])
        pos = pos - dir*(float)dt*0.5f; */

    dir = dir.Cross(vec3(0, 1, 0));
    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
        pos = pos + dir*(float)dt*0.5f; 
        animating = true;
    }
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
        pos = pos - dir*(float)dt*0.5f; 
        animating = true;
    }
    trans->SetPosition(pos);

    
    if (keys[SDL_SCANCODE_K])
    {
        g_renderer.light.direction = RotateY((float)dt) * g_renderer.light.direction;
        Resize(g_renderer.GetWidth(), g_renderer.GetHeight());
    }
    if (keys[SDL_SCANCODE_J])
    {
        g_renderer.light.direction = RotateY(-(float)dt) * g_renderer.light.direction;
        Resize(g_renderer.GetWidth(), g_renderer.GetHeight());
    }

    if (keys[SDL_SCANCODE_Q])
        angle += (float)dt;
    if (keys[SDL_SCANCODE_E])
        angle -= (float)dt;

    for (size_t i=0; i<g_systems.size(); ++i)
        g_systems[i]->Update(dt);

    if (g_stickmesh && animating)
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
    MeshRenderSystem<ToonMaterial> toonRenderSystem(&g_renderer);
    g_systems.push_back(&diffuseRenderSystem);
    g_systems.push_back(&specularRenderSystem);
    g_systems.push_back(&toonRenderSystem);

    // Create some entities
    g_entities.resize(10);
    
    // MeshComponent<MaterialType> is a component to store a mesh and a material
    typedef MeshComponent<DiffuseMaterial> DiffuseMeshComponent;  // DiffuseMaterial supports diffuse color, texture and shadow on surface
    typedef MeshComponent<SpecularMaterial> SpecularMeshComponent; // SpecularMaterial also supports a specular color and shininess
    typedef MeshComponent<ToonMaterial> ToonMeshComponent;          // ToonMaterial only supports a color and performs toon shading

//#define TOON_SHADING
    
    // Stickman entity, with mesh loaded from file
#ifdef TOON_SHADING
    auto msc = g_entities[0].AddComponent<ToonMeshComponent>(0.15f);
#else
    auto msc = g_entities[0].AddComponent<SpecularMeshComponent>(0.15f);
    msc->material.depthBias = 0.05f;
    msc->material.shininess = 20.0f;
    msc->material.specularColor = vec3(1.0f, 1.0f, 1.0f);
#endif
    g_stickmesh = &msc->mesh;
    msc->mesh.LoadAnimatedFile("test1.dat");
    g_entities[0].AddComponent<TransformComponent>(vec3(0,0.07f,0), vec3(-90*3.1415f/180.0f,0,0));
    
    // Ground entity, with box mesh and green diffuse color
#ifdef TOON_SHADING
    auto mc = g_entities[1].AddComponent<ToonMeshComponent>();
#else
    auto mc = g_entities[1].AddComponent<DiffuseMeshComponent>();
    mc->material.depthBias = 0.0f;
#endif
    mc->material.diffuseColor = vec3(0.0f, 1.0f, 0.0f);
    mc->mesh.LoadBox(3.0f, 0.05f, 3.0f);        // Larger than this ground size seems to give problems while shadow mapping; so use smaller pieces of ground entities instead of one large box
    g_entities[1].AddComponent<TransformComponent>(vec3(0,-1.05f,0));
    
    // Cube entity, with box mesh and texture loaded from file
#ifdef TOON_SHADING
    mc = g_entities[2].AddComponent<ToonMeshComponent>(1.0f);
    mc->material.diffuseColor = vec3(0.0f, 0.0f, 1.0f);
#else
    mc = g_entities[2].AddComponent<DiffuseMeshComponent>(1.0f);
    mc->material.depthBias = 0.008f;
    mc->material.textureId = g_textureManager.AddTexture("grass_T.bmp");
#endif
    mc->mesh.LoadBox(0.5f, 0.5f, 0.5f);
    //mc->mesh.LoadCone(0.2f, 1.0f, 20);
    g_entities[2].AddComponent<TransformComponent>(vec3(2,-0.5f,-1));

    // A camera entity
    g_entities[3].AddComponent<CameraComponent>();
    auto t = g_entities[3].AddComponent<TransformComponent>();
    t->SetTransform(LookAt(vec3(-3, 2.0f, -3.0f), vec3(0,1,0), vec3(0,1,0)).AffineInverse());
 
    // Test transparent entity
#ifdef TOON_SHADING
    msc = g_entities[4].AddComponent<ToonMeshComponent>();
#else
    msc = g_entities[4].AddComponent<SpecularMeshComponent>();
    msc->material.depthBias = 0.0f;
    msc->material.shininess = 20.0f;
    msc->material.specularColor = vec3(1.0f, 1.0f, 1.0f);
#endif
    msc->material.diffuseColor = vec4(1, 0, 0, 0.4f);
    msc->mesh.LoadSphere(0.5f, 30, 30);
    //msc->mesh.LoadBox(0.5f, 0.5f, 0.5f);
    msc->transparent = true;
    g_entities[4].AddComponent<TransformComponent>(vec3(-1.05f, 0, 0));



#ifdef TOON_SHADING
    mc = g_entities[5].AddComponent<ToonMeshComponent>(1.0f);
#else
    mc = g_entities[5].AddComponent<DiffuseMeshComponent>(1.0f);
    mc->material.depthBias = 0.008f;
#endif
    mc->material.diffuseColor = vec3(0.0f, 0.0f, 1.0f);
    mc->mesh.LoadCone(0.4f, 1.0f, 20);
    g_entities[5].AddComponent<TransformComponent>(vec3(2,-1.0f,0));




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
