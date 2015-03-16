extern Renderer g_renderer;
extern TextureManager g_textureManager;


#ifdef SPECULAR_SHADERS
class SpecularShaders
#else
class DiffuseShaders
#endif
{
public:
    struct Uniforms
    {
        float depthBias;
        size_t textureId;
        vec3 diffuseColor;
#ifdef SPECULAR_SHADERS
        vec3 specularColor;
        float shininess;
#endif
    };
    static Uniforms uniforms;

#ifdef ATTRIBUTES_NUM
#undef ATTRIBUTES_NUM
#endif

#ifdef SPECULAR_SHADERS
#define ATTRIBUTES_NUM 4
#else
#define ATTRIBUTES_NUM 3
#endif

    // VertexShader is called for each vertex and is expected to return its
    //  position in clip space as well as its attributes
    static vec4 VertexShader(vec4 attribute[], const Vertex& vertex)
    {
        vec4 p = g_renderer.transforms.mvp * vec4(vertex.position);
        attribute[0] = mat3(g_renderer.transforms.model) * vertex.normal;
        attribute[1] = vertex.texcoords;
        
        // Also take to light space; for shadow map calculations
        attribute[2] = g_renderer.transforms.bias_light_mvp * vec4(vertex.position);
        attribute[2] = attribute[2].ConvertToVec3();
        attribute[2].x = (attribute[2].x) * (float)g_renderer.GetWidth();
        attribute[2].y = (attribute[2].y) * (float)g_renderer.GetHeight();

#ifdef SPECULAR_SHADERS
        attribute[3] = g_renderer.transforms.model * vec4(vertex.position);
#endif
        return p;
    }

    // Get depth sample from the depthbuffer with light-space x,y-coordinates
    // This gives closest depth to the light
    static float GetSample(float x, float y)
    {
        float sample = 1.0f;
        size_t samplingPos = int(y)*g_renderer.GetWidth() + (int)x;
        if (samplingPos < g_renderer.GetWidth()*g_renderer.GetHeight())
            sample = g_renderer.GetDepthBuffer(1)[samplingPos];
        return sample;
    }
    
    // This function is called for each pixel
    // The Point contains x,y position of the pixel,
    //  the depth value and the interpolated attributes
    static void FragmentShader(Point<ATTRIBUTES_NUM>& point)
    {
        // Code here may need to be optimized
        
        // Get normal and texture-color for the pixel
        vec3 n = point.attribute[0];
        n.Normalize();
        vec3 texcolor = g_textureManager.GetTexture(uniforms.textureId).Sample(point.attribute[1]);
        
        // Perform a simple phong based lighting calculation for directional light
        // Ambient Lighting:
        vec3 c = g_renderer.light.ambient;
        
        vec3 dir = g_renderer.light.direction;                   // assuming this is normalized
        float diffuseFactor = n.Dot(-dir);

        if (diffuseFactor > 0)
        {
            // Diffuse Lighting:
            c = c + uniforms.diffuseColor * diffuseFactor * g_renderer.light.diffuse;

#ifdef SPECULAR_SHADERS
            vec3 view = g_renderer.transforms.camPos - point.attribute[3];
            view.Normalize();
            // Specular Lighting:
            float specintensity = dir.Reflect(n).Dot(view);
            if (specintensity > 0.0f)
            {
                specintensity = (float) pow(specintensity, uniforms.shininess);
                c = c + uniforms.specularColor * specintensity * g_renderer.light.specular;
            }
#endif
        }
        c.x = Min(c.x, 1.0f);
        c.y = Min(c.y, 1.0f);
        c.z = Min(c.z, 1.0f);
        
        // Shadow Mapping
        // Light space position of pixel
        vec3 lpos = point.attribute[2];
   
        float visibility = 1.0f;
        // Compare light space depth of this pixel with
        // sample depth of this and nearby pixels from depthbuffer
        for (float i=-1.5f; i<=1.5f; i+=1.5f)
            for (float j=-1.5f; j<=1.5f; j+=1.5f)
                if (GetSample(lpos.x + i, lpos.y +j) < lpos.z - uniforms.depthBias)
                    visibility -= 0.06f;
        c = c * visibility;
    
        c = c * texcolor;
        g_renderer.PutPixel(point.pos[0], point.pos[1], c);     // Use the calculated color to plot the pixel
    }

    typedef Shaders<g_renderer, Vertex, ATTRIBUTES_NUM, &VertexShader, &FragmentShader> ShadersType;
    static ShadersType shaders;
    //              Shaders<Renderer&, VertexClass, NumberOfAttributes, VertexShaderFunction, FragmentShaderFunction>
};
