#pragma once
#include "Renderer.h"

struct Vertex
{
    vec3 position, normal;
    vec2 texcoords;
};


// Class to store vertex and index buffers
class Mesh
{
public:
    Mesh();
    // Load mesh from a file
    void LoadFile(const std::string &filename);
    // Load a box as the mesh
    void LoadBox(float halfLength, float halfHeight, float halfWidth);
    // Load a sphere as the mesh
    void LoadSphere(float radius, uint16_t rings, uint16_t sectors);
    
    // Draw the mesh with the given shaders
    template<class ShadersClass>
    void Draw(ShadersClass &shaders)
    {
        shaders.DrawTriangles(m_vertices, m_indices);
    }
    
    // Get index of texture associated with this mesh
    size_t GetTextureId() { return m_textureId; }
    // Associate this mesh with texture of given index
    void SetTextureId(size_t textureId) { m_textureId = textureId; }
private:
    std::vector<Vertex> m_vertices;     // Vertex Buffer
    std::vector<uint16_t> m_indices;    // Index Buffer
    size_t m_textureId;                 // Texture Id of the mesh
};
