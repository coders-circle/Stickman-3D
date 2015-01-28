#include "Mesh.h"
#include "shaders/shaders3d.h"

struct TextureShadowMaterial : public Material
{
    TextureShadowMaterial() : depthBias(0.007f), textureId(0) {}
    size_t textureId;
    float depthBias;

    void DrawMesh(Mesh<TextureShadowMaterial>& mesh)
    {
        TextureShadowShaders::Uniforms &uniforms = TextureShadowShaders::uniforms;
        uniforms.depthBias = depthBias;
        uniforms.textureId = textureId;
        mesh.Draw(TextureShadowShaders::shaders);
    }
};
