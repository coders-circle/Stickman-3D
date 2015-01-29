#pragma once
#include <matrix.h>

enum COMPONENT_TYPE
{
    TRANSFORM_COMPONENT,
    MESH_COMPONENT,
    CAMERA_COMPONENT,
};

struct ComponentBase
{
};
template <COMPONENT_TYPE componentType>
struct Component : public ComponentBase
{
    static COMPONENT_TYPE GetType() { return componentType; }
};

struct TransformComponent: public Component<TRANSFORM_COMPONENT>
{
    TransformComponent(const vec3 &position=vec3(), const vec3 &rotation=vec3()) : rot(rotation), pos(position) { UpdateTransform(); }
    void UpdateTransform()
    {
        transform = Translate(pos) * EulerXYZ(rot);
        isdirty = false;
    }
    const mat4 GetTransform()
    {
        if (isdirty)
            UpdateTransform();
        return transform;
    }
    

    float GetRotationX() const { return rot.x; }
    float GetRotationY() const { return rot.y; }
    float GetRotationZ() const { return rot.z; }
    const vec3& GetRotation() const { return rot; }
    const vec3& GetPosition() const { return pos; }

    void SetRotationX(float angle) { rot.x = angle; isdirty = true; }
    void SetRotationY(float angle) { rot.y = angle; isdirty = true; }
    void SetRotationZ(float angle) { rot.z = angle; isdirty = true; }
    void SetRotation(const vec3& rotation) { rot = rotation; isdirty = true; }
    void SetPosition(const vec3& position) { pos = position; isdirty = true; }

private:
    vec3 pos;
    vec3 rot;
    bool isdirty;
    mat4 transform;
};

#include <Mesh.h>

struct Material
{};

template <class MaterialClass>
struct MeshComponent : public Component<MESH_COMPONENT>
{
    // Compile-time test for correct Material Class : "MaterialClass" must be derived from Material
    static_assert(std::is_base_of<Material, MaterialClass>::value, "Invalid Material Class");

    MeshComponent(const Mesh& mesh, float scale=1.0f) : mesh(mesh), scale(scale) {}
    MeshComponent(float scale=1.0f) : scale(scale) {}
    Mesh mesh; 
    MaterialClass material;
    float scale;
};
