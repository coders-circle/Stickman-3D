#pragma once

extern Renderer g_renderer;
extern TextureManager g_textureManager;
   
class TextureShadowShaders
{
public:
    struct Uniforms
    {
        float depthBias;
        size_t textureId;
    };
    static Uniforms uniforms;

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
    static void FragmentShader(Point<3>& point)
    {
        // Code here may need to be optimized
        
        // Get normal and color for the pixel
        vec3 n = point.attribute[0];
        n.Normalize();
        vec3 c = g_textureManager.GetTexture(uniforms.textureId).Sample(point.attribute[1]);
        
        // Light space position of pixel
        vec3 lpos = point.attribute[2];
   
        float visibility = 1.0f;
        // Compare light space depth of this pixel with
        // sample depth of this and nearby pixels from depthbuffer
        for (float i=-1.5f; i<=1.5f; i+=1.5f)
            for (float j=-1.5f; j<=1.5f; j+=1.5f)
                if (GetSample(lpos.x + i, lpos.y +j) < lpos.z - uniforms.depthBias)
                    visibility -= 0.06f;
        //visibility = Max(0.0f, visibility);
        
        c = c * visibility;
    
        // Perform a simple phong based lighting calculation for directional light
        vec3 dir = g_renderer.lights.lightDirection;                   // assuming this is normalized
        float intensity = Min(Max(n.Dot(-dir), 0.0f) + 0.2f, 1.0f);
        c = c*intensity;    // "Light" the color
        
        g_renderer.PutPixel(point.pos[0], point.pos[1], c);     // Use the calculated color to plot the pixel
    }

    typedef Shaders<g_renderer, Vertex, 3, &VertexShader, &FragmentShader> ShadersType;
    static ShadersType shaders;
    //              Shaders<Renderer&, VertexClass, NumberOfAttributes, VertexShaderFunction, FragmentShaderFunction>
};
