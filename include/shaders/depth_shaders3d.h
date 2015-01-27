#pragma once

vec4 VertexDepthShader(vec4 attribute[], const Vertex& vertex)
{
    vec4 p = transform * vec4(vertex.position);
    return p;
}

void FragmentDepthShader(Point<0>& point)
{}

auto shadersDepth = 
                Shaders<g_renderer, Vertex, 0, &VertexDepthShader, &FragmentDepthShader, true>(); 
                                                                                        // backface visible/frontface culling = true

