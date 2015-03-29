extern Renderer g_renderer;


class CellShaders
{
public:
    struct Uniforms
    {
        vec4 diffuseColor;
    };
    static Uniforms uniforms;

    static vec4 VertexShader(vec4 attribute[], const Vertex& vertex)
    {
        vec4 p = g_renderer.transforms.mvp * vec4(vertex.position);
        attribute[0] = mat3(g_renderer.transforms.model) * vertex.normal;
        return p;
    }
 
    static void FragmentShader(Point<1>& point)
    {
        vec3 n = point.attribute[0];
        n.Normalize();
        
        vec3 c = g_renderer.light.ambient;
        
        vec3 dir = g_renderer.light.direction;
        float diffuseFactor = n.Dot(-dir);
        
        if (diffuseFactor > 0.5f)
            c = uniforms.diffuseColor * 0.7f;
        else
            c = uniforms.diffuseColor * 0.6f;

        c = c* g_renderer.light.diffuse;
        c = c+ g_renderer.light.ambient;
        
        c.x = Min(c.x, 1.0f);
        c.y = Min(c.y, 1.0f);
        c.z = Min(c.z, 1.0f);
        
        g_renderer.PutPixelUnsafe(point.pos[0], point.pos[1], c, 1.0f);     // Use the calculated color to plot the pixel
    }

    typedef Shaders<g_renderer, Vertex, 1, &VertexShader, &FragmentShader> ShadersType;
    static ShadersType shaders;
    //              Shaders<Renderer&, VertexClass, NumberOfAttributes, VertexShaderFunction, FragmentShaderFunction>
};
