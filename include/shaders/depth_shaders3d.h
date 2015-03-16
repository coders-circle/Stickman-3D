#pragma once

// Shaders for depth buffer generation

extern Renderer g_renderer;

vec4 VertexDepthShader(vec4 attribute[], const Vertex& vertex)
{   
    // Since only depth is needed
    // no additional attributes are calculated
    vec4 p = g_renderer.transforms.mvp * vec4(vertex.position);
    return p;
}

void FragmentDepthShader(Point<0>& point)
{
    // We don't need to process the pixels
    // as depth is automatically stored in depth buffer by the rasterizer
}

auto shadersDepth = 
                Shaders<g_renderer, Vertex, 0, &VertexDepthShader, &FragmentDepthShader, true>(); 
                                                                                        // backface visible/frontface culling = true

