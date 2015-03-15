#pragma once
#include "Renderer.h"
#include "TextureManager.h"
#include "animation.h"

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
    ~Mesh();

    //Load animated mesh from a file
    void LoadAnimatedFile(const std::string &filename);
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
        if (m_animation)
        {
            for (size_t i=0; i<m_vertices.size(); ++i)
            {
                WeightInfo& wt = m_animation->skin[i];
                mat4 t;
                for (int k=0; k<6; ++k)
                {
                    if (wt.weights[k] <= 0)
                        continue;
                    Bone& bn = m_animation->bones[wt.boneids[k]];
                    t = t + (bn.node->combined_transform * bn.offset) * wt.weights[k];
                }
                m_animation->tempVertices[i] = m_vertices[i];
                m_animation->tempVertices[i].position = t*m_vertices[i].position;
            }
            shaders.DrawTriangles(m_animation->tempVertices, m_indices, transparency);
        }
        else
            shaders.DrawTriangles(m_vertices, m_indices, transparency);
    }
    
    const Animation* GetAnimation() const { return &m_animation->animation; }
    void Animate(double time);

private:
    std::vector<Vertex> m_vertices;     // Vertex Buffer
    std::vector<uint16_t> m_indices;    // Index Buffer
    
    struct AnimationInfo
    {
        std::vector<Bone> bones;
        Node root;
        Animation animation;
        std::vector<WeightInfo> skin;
        std::vector<Vertex> tempVertices;

        std::map<unsigned int, Node*> map;
    } * m_animation;

    void ReadNode(std::fstream& file, Node* node);

    void UpdateNode(Node& node, Node* parent=NULL);
};
