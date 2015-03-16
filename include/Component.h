#pragma once
#include <matrix.h>

enum COMPONENT_TYPE
{
    TRANSFORM_COMPONENT,
    CAMERA_COMPONENT,
    
    MESH_COMPONENT,
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
    mat4 GetTransform()
    {
        if (isdirty)
            UpdateTransform();
        return transform;
    }
    void SetTransform(const mat4& transform)
    {
        this->transform = transform;
        isdirty = false;
        pos = transform.Column(3);
        rot.x = atan2f(transform[2][1], transform[2][2]);
        rot.y = atan2f(-transform[2][0], sqrtf(transform[2][1]*transform[2][1] + transform[2][2]*transform[2][2]));
        rot.z = atan2f(transform[1][0], transform[0][0]);
    }

    const vec3& GetRotation() const { return rot; }
    const vec3& GetPosition() const { return pos; }

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
struct MeshComponent : public Component<COMPONENT_TYPE(MESH_COMPONENT+MaterialClass::ID)>
{
    // Compile-time test for correct Material Class : "MaterialClass" must be derived from Material
    static_assert(std::is_base_of<Material, MaterialClass>::value, "Invalid Material Class");

    MeshComponent(const Mesh& mesh, float scale=1.0f, bool transparent=false) : mesh(mesh), scale(scale), transparent(transparent) {}
    MeshComponent(float scale=1.0f, bool transparent=false) : scale(scale), transparent(transparent) {}
    Mesh mesh; 
    MaterialClass material;
    float scale;
    bool transparent;
};

class CameraSystem;
struct CameraComponent : public Component<CAMERA_COMPONENT>
{
    CameraComponent(float fov = 60, float near = 0.01f, float far = 100.0f)
        : fov(fov), near(near), far(far) {}
    float fov, near, far;

    friend CameraSystem;
private:
    mat4 projection;
};
