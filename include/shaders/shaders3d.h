#pragma once

// VertexShader is called for each vertex and is expected to return its
//  position in clip space as well as its attributes
vec4 VertexShader(vec4 attribute[], const Vertex& vertex)
{
    vec4 p = transform * vec4(vertex.position);
    attribute[0] = mat3(model) * vertex.normal;
    attribute[1] = vertex.texcoords;

    // Take to light space
    attribute[2] = bias_light_mvp * vec4(vertex.position);
    attribute[2] = attribute[2].ConvertToVec3();
    attribute[2].x = (attribute[2].x) * g_renderer.GetWidth();
    attribute[2].y = (attribute[2].y) * g_renderer.GetHeight();
    return p;
}

// This function is called for each pixel
// The Point contains x,y position of the pixel,
//  the depth value and the interpolated attributes
void FragmentShader(Point<3>& point)
{
    vec3 n = point.attribute[0];
    n.Normalize();
    vec3 c = g_textureManager.GetTexture(texId).Sample(point.attribute[1]);
    
    vec3 lpos = point.attribute[2];

    float sample = 1.0f;
    size_t samplingPos = int(lpos.y)*g_renderer.GetWidth() + (int)lpos.x;
    if (samplingPos < g_renderer.GetWidth()*g_renderer.GetHeight())
        sample = g_renderer.GetDepthBuffer(1)[samplingPos];

    if (sample - lpos.z < -0.01f)
        c = c*0.1f;

    // Perform a simple phong based lighting calculation for directional light
    vec3 dir = lightDir;                   
    dir.Normalize();
    float intensity = Min(Max(n.Dot(-dir), 0.0f) + 0.2f, 1.0f);
    c = c*intensity;    // "Light" the color
    
    g_renderer.PutPixel(point.pos[0], point.pos[1], c);     // Use the calculated color to plot the pixel
}

auto shaders = 
                Shaders<g_renderer, Vertex, 3, &VertexShader, &FragmentShader>();
//              Shaders<Renderer&, VertexClass, NumberOfAttributes, VertexShaderFunction, FragmentShaderFunction>

