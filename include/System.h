#pragma once
#include <Entity.h>

class SystemBase
{
public:
    virtual void Initialize() {};
    virtual void CleanUp() {};
    virtual void Update(double dt) {};
    virtual void RenderShadow() {};
    virtual void Render() {};
    virtual void PostRender() {};
    virtual void Resize(int width, int height) {};

    virtual void AddEntity(Entity* entity) { m_entities.push_back(entity); };

protected:
    std::vector<Entity*> m_entities;
};

template<class... Args>
class System : public SystemBase
{
public:
    void AddEntity(Entity* entity)
    {
        if (!CheckEntity<Args...>(entity))
            return;
        m_entities.push_back(entity);
    }
    
private:
    template<class T1>
    bool CheckEntity(Entity* entity)
    {
        if (!entity->HasComponent<T1>())
            return false;
        return true;
    }
    
    template<class T1, class T2, class... Ts>
    bool CheckEntity(Entity* entity)
    {
        if (!CheckEntity<T1>(entity))
            return false;
        return CheckEntity<T2, Ts...>(entity);
    }
    
};

extern mat4 bias_matrix;

template <class T>
class MeshRenderSystem : public System<TransformComponent, MeshComponent<T>>
{
public:
    MeshRenderSystem(Renderer* renderer) : m_renderer(renderer) {}
    
    void RenderShadow()
    {
        for (size_t i=0; i< SystemBase::m_entities.size(); ++i)
        {
            Entity* entity = SystemBase::m_entities[i];
            auto mc = entity->GetComponent<MeshComponent<T>>();
            m_renderer->transforms.model = entity->GetComponent<TransformComponent>()->GetTransform()
                                            * Scale(mc->scale);
            m_renderer->transforms.mvp = m_renderer->transforms.light_vp * m_renderer->transforms.model;
            mc->mesh.Draw(shadersDepth);
        }

     
    }
    void Render()
    {
        for (size_t i=0; i<SystemBase::m_entities.size(); ++i)
        {
            Entity* entity = SystemBase::m_entities[i];
            auto mc = entity->GetComponent<MeshComponent<T>>();
            if (mc->transparent)
                continue;
            m_renderer->transforms.model = entity->GetComponent<TransformComponent>()->GetTransform()
                                            * Scale(mc->scale);
            m_renderer->transforms.mvp = m_renderer->transforms.vp * m_renderer->transforms.model;
            m_renderer->transforms.bias_light_mvp = bias_matrix * m_renderer->transforms.light_vp * m_renderer->transforms.model;
            mc->material.DrawMesh(mc->mesh);
        }
    }

    void PostRender()
    {
        for (size_t i=0; i<SystemBase::m_entities.size(); ++i)
        {
            Entity* entity = SystemBase::m_entities[i];
            auto mc = entity->GetComponent<MeshComponent<T>>();
            if (!mc->transparent)
                continue;
            m_renderer->transforms.model = entity->GetComponent<TransformComponent>()->GetTransform()
                                            * Scale(mc->scale);
            m_renderer->transforms.mvp = m_renderer->transforms.vp * m_renderer->transforms.model;
            m_renderer->transforms.bias_light_mvp = bias_matrix * m_renderer->transforms.light_vp * m_renderer->transforms.model;
            mc->material.DrawMesh(mc->mesh, true);
        }
    }

private:
    Renderer* m_renderer;
};

class CameraSystem : public System<TransformComponent, CameraComponent>
{
public:
    CameraSystem(Renderer* renderer) : m_renderer(renderer), m_activeCamera(0) {}

    void SetActiveCamera(size_t cameraId) { m_activeCamera = cameraId; }
    size_t GetActiveCamera() const { return m_activeCamera; }

    void Render()
    {
        if (m_activeCamera >= m_entities.size())
            return;
        auto cam = m_entities[m_activeCamera]->GetComponent<CameraComponent>();
        auto trans = m_entities[m_activeCamera]->GetComponent<TransformComponent>();
        mat4 view  = trans->GetTransform().AffineInverse();
        mat4 proj = cam->projection;
        m_renderer->transforms.vp = proj * view;
        m_renderer->transforms.camPos = trans->GetPosition();
    }

    void Resize(int width, int height)
    {
        for (size_t i=0; i<m_entities.size(); ++i)
        {
            auto cam = m_entities[i]->GetComponent<CameraComponent>();
            auto trans = m_entities[i]->GetComponent<TransformComponent>();
            cam->projection = Perspective(cam->fov*3.1415f/180, float(width)/float(height), cam->near, cam->far);
        }
    }
private:
    Renderer* m_renderer;
    size_t m_activeCamera;
};
