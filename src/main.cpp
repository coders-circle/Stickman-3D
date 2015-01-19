#include <common.h>
#include <Renderer.h>
#include <transform.h>
#include <Bitmap.h>

Renderer g_renderer;
Bitmap bmp;
// Transformations
mat4 transform, model, persp;

// This struct is input to the vertex shader
struct Vertex
{
    vec3 v;
    vec3 n;
    vec2 uv;
};


// VertexShader is called for each vertex and is expected to return
//  position of vertex in clip space and the varying attributes
// The varyings are attributes that vary for each pixel
//  These attributes are interpolated for intermediate pixels between the vertices
//  The number of varying depends on the input of the FragmentShader 
//  used with this VertexShader
vec4 VertexShader(vec4 varying[], const Vertex& vertex)
{
    vec4 p = transform * vec4(vertex.v);        // Transform the vertex by composite ModelViewProjection matrix
    varying[0] = mat3(model) * vertex.n;              // Transform the normal by model matrix
    varying[1] = vertex.uv;
    return p;                                   // Return the position of the vertex in clip-space
}

// Point<N> for point with N-varying attributes
// This function is called for each pixel
// The Point contains x,y position of the pixel,
// the depth value and the interpolated varying attributes
void FragmentShader(Point<2>& point)
{
    vec3 n = point.varying[0];  // Take in the interpolated normal
    n.Normalize();              //  and normalize it
        
    vec3 c = bmp.Sample(point.varying[1]);   // Get color by sampling the bitmap    

    // Perform a simple phong based lighting calculation for directional light
    vec3 dir(-1,0,-1);                   
    dir.Normalize();
    float intensity = Min(Max(n.dot(-dir), 0.0f) + 0.05f, 1.0f);
    c = c*intensity;    // "Light" the color
    
    g_renderer.PutPixel(point.pos[0], point.pos[1], c);     // Use the calculated color to plot the pixel
}

// Vertex Buffer (CUBE)
Vertex vertices[] = 
{
    // FRONT
    { vec3(-0.5f,  0.5f,  0.5f), vec3( 0,  0,  1), vec2(0.0f, 0.0f) },
    { vec3( 0.5f,  0.5f,  0.5f), vec3( 0,  0,  1), vec2(1.0f, 0.0f) },
    { vec3(-0.5f, -0.5f,  0.5f), vec3( 0,  0,  1), vec2(0.0f, 1.0f) },
    { vec3( 0.5f, -0.5f,  0.5f), vec3( 0,  0,  1), vec2(1.0f, 1.0f) },
    // RIGHT                           
    { vec3( 0.5f,  0.5f,  0.5f), vec3( 1,  0,  0), vec2(0.0f, 0.0f) },
    { vec3( 0.5f,  0.5f, -0.5f), vec3( 1,  0,  0), vec2(1.0f, 0.0f) },
    { vec3( 0.5f, -0.5f,  0.5f), vec3( 1,  0,  0), vec2(0.0f, 1.0f) },
    { vec3( 0.5f, -0.5f, -0.5f), vec3( 1,  0,  0), vec2(1.0f, 1.0f) },
    // LEFT
    { vec3(-0.5f,  0.5f, -0.5f), vec3(-1,  0,  0), vec2(0.0f, 0.0f) },
    { vec3(-0.5f,  0.5f,  0.5f), vec3(-1,  0,  0), vec2(1.0f, 0.0f) },
    { vec3(-0.5f, -0.5f, -0.5f), vec3(-1,  0,  0), vec2(0.0f, 1.0f) },
    { vec3(-0.5f, -0.5f,  0.5f), vec3(-1,  0,  0), vec2(1.0f, 1.0f) },
    // TOP
    { vec3(-0.5f,  0.5f, -0.5f), vec3( 0,  1,  0), vec2(0.0f, 0.0f) },
    { vec3( 0.5f,  0.5f, -0.5f), vec3( 0,  1,  0), vec2(1.0f, 0.0f) },
    { vec3(-0.5f,  0.5f,  0.5f), vec3( 0,  1,  0), vec2(0.0f, 1.0f) },
    { vec3( 0.5f,  0.5f,  0.5f), vec3( 0,  1,  0), vec2(1.0f, 1.0f) },
    // BOTTOM
    { vec3(-0.5f, -0.5f,  0.5f), vec3( 0, -1,  0), vec2(0.0f, 0.0f) },
    { vec3( 0.5f, -0.5f,  0.5f), vec3( 0, -1,  0), vec2(1.0f, 0.0f) },
    { vec3(-0.5f, -0.5f, -0.5f), vec3( 0, -1,  0), vec2(0.0f, 1.0f)},
    { vec3( 0.5f, -0.5f, -0.5f), vec3( 0, -1,  0), vec2(1.0f, 1.0f) },
    // BACK
    { vec3( 0.5f,  0.5f, -0.5f), vec3( 0,  0, -1), vec2(0.0f, 0.0f) },
    { vec3(-0.5f,  0.5f, -0.5f), vec3( 0,  0, -1), vec2(1.0f, 0.0f) },
    { vec3( 0.5f, -0.5f, -0.5f), vec3( 0,  0, -1), vec2(0.0f, 1.0f) },
    { vec3(-0.5f, -0.5f, -0.5f), vec3( 0,  0, -1), vec2(1.0f, 1.0f) },
};

// Index Buffer (CUBE)
uint16_t indices[] = 
{
    0, 1, 3, 0, 3, 2,
    4, 5, 7, 4, 7, 6,
    8, 9, 11, 8, 11, 10,
    12, 13, 15, 12, 15, 14,
    16, 17, 19, 16, 19, 18,
    20, 21, 23, 20, 23, 22
};


// angle of rotation of the object
float angle=45.0f*3.1415/180.0f;
void Render()
{
    // Before rendering, store the model and composite ModelViewProjection matrices
    // so that vertex shader may use these
    model = Translate(vec3(0,0,-3))*RotateY(angle);
    transform = persp * model;  

    // Draw the triangles with given vertices, indices, VertexShader and FragmentShader
    g_renderer.DrawTriangles(&VertexShader, &FragmentShader, vertices, 24, indices, 12);
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

int main()
{
    g_renderer.Initialize("Test Renderer", 100, 100, 800, 600);
    g_renderer.SetClearColor(RGBColor(100, 149, 237));
    g_renderer.SetRenderCallback(&Render);
    g_renderer.SetUpdateCallback(&Update);
    g_renderer.SetResizeCallback(&Resize);
    
    // Load the bitmap
    bmp.LoadFile("grass_T.bmp");
    
    // Call resize once to initialize the projection matrix
    Resize(g_renderer.GetWidth(), g_renderer.GetHeight());
    
    g_renderer.MainLoop();

    g_renderer.CleanUp();
        return 0;
}
