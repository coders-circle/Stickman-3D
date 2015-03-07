#pragma once
#include "Renderer.h"
#include "TextureManager.h"

struct Vertex
{
    vec3 position, normal;
    vec2 texcoords;
};

// Class to store vertex and index buffers
class Mesh
{
public:
    // Load mesh from a file
    void LoadFile(const std::string &filename);
    // Load a box as the mesh
    void LoadBox(float halfLength, float halfHeight, float halfWidth);
    // Load a sphere as the mesh
    void LoadSphere(float radius, uint16_t rings, uint16_t sectors);
    
    // Draw the mesh with the given shaders
    template<class ShadersClass>
    void Draw(ShadersClass &shaders, bool transparency=false)
    {
        shaders.DrawTriangles(m_vertices, m_indices, transparency);
    }

private:
    std::vector<Vertex> m_vertices;     // Vertex Buffer
    std::vector<uint16_t> m_indices;    // Index Buffer
};
