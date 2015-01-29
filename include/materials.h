#include "Mesh.h"
#include "shaders/shaders3d.h"

struct TextureShadowMaterial : public Material
{
    TextureShadowMaterial() : depthBias(0.007f), textureId(0), diffuseColor(vec3(1.0f, 1.0f, 1.0f)) {}
    size_t textureId;
    float depthBias;
    vec3 diffuseColor;

    void DrawMesh(Mesh& mesh)
    {
        TextureShadowShaders::Uniforms &uniforms = TextureShadowShaders::uniforms;
        uniforms.depthBias = depthBias;
        uniforms.textureId = textureId;
        uniforms.diffuseColor = diffuseColor;
        mesh.Draw(TextureShadowShaders::shaders);
    }
};
