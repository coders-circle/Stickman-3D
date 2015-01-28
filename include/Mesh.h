#pragma once
#include "Renderer.h"
#include "TextureManager.h"

struct Material
{};

struct Vertex
{
    vec3 position, normal;
    vec2 texcoords;
};

// Class to store vertex and index buffers
template <class MaterialClass>
class Mesh
{
public:
    // Compile-time test for correct Material Class : "MaterialClass" must be derived from Material
    static_assert(std::is_base_of<Material, MaterialClass>::value, "Invalid Material Class");

    // Load mesh from a file
    void LoadFile(const std::string &filename);
    // Load a box as the mesh
    void LoadBox(float halfLength, float halfHeight, float halfWidth);
    // Load a sphere as the mesh
    void LoadSphere(float radius, uint16_t rings, uint16_t sectors);
    
    // Draw the mesh with its material
    void Draw()
    {
        material.DrawMesh(*this);
    }

    // Draw the mesh with the given shaders
    template<class ShadersClass>
    void Draw(ShadersClass &shaders)
    {
        shaders.DrawTriangles(m_vertices, m_indices);
    }

    MaterialClass material;
    
private:
    std::vector<Vertex> m_vertices;     // Vertex Buffer
    std::vector<uint16_t> m_indices;    // Index Buffer
};
