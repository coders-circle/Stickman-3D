#pragma once

// VertexShader is called for each vertex and is expected to return its
//  position in clip space as well as its attributes
vec4 VertexDepthShader(vec4 attribute[], const Vertex& vertex)
{
    vec4 p = transform * vec4(vertex.position);
    return p;
}

// This function is called for each pixel
// The Point contains x,y position of the pixel,
//  the depth value and the interpolated attributes
void FragmentDepthShader(Point<0>& point)
{}

auto shadersDepth = 
                Shaders<g_renderer, Vertex, 0, &VertexDepthShader, &FragmentDepthShader, true>();
//              Shaders<Renderer&, VertexClass, NumberOfAttributes, VertexShaderFunction, FragmentShaderFunction>

