#include "Mesh.h"
#include "shaders.h"

struct DiffuseMaterial : public Material
{
    DiffuseMaterial() : depthBias(0.007f), textureId(0), diffuseColor(vec4(1.0f, 1.0f, 1.0f, 1.0f)) {}
    size_t textureId;
    float depthBias;
    vec4 diffuseColor;
    static const int ID = 0;

    void DrawMesh(Mesh& mesh, bool transparency=false)
    {
        DiffuseShaders::Uniforms &uniforms = DiffuseShaders::uniforms;
        uniforms.depthBias = depthBias;
        uniforms.textureId = textureId;
        uniforms.diffuseColor = diffuseColor;
        mesh.Draw(DiffuseShaders::shaders, transparency);
    }
};


struct SpecularMaterial : public Material
{
    SpecularMaterial() : depthBias(0.007f), textureId(0), diffuseColor(vec4(1.0f, 1.0f, 1.0f, 1.0f)), specularColor(vec3(1.0f, 1.0f, 1.0f)),
                        shininess(32.0f) {}
    size_t textureId;
    float depthBias;
    vec4 diffuseColor;
    vec3 specularColor;
    float shininess;
    static const int ID = 1;

    void DrawMesh(Mesh& mesh, bool transparency=false)
    {
        SpecularShaders::Uniforms &uniforms = SpecularShaders::uniforms;
        uniforms.depthBias = depthBias;
        uniforms.textureId = textureId;
        uniforms.diffuseColor = diffuseColor;
        uniforms.specularColor = specularColor;
        uniforms.shininess = shininess;
        mesh.Draw(SpecularShaders::shaders, transparency);
    }
};
