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

extern mat4 light_vp, bias_matrix, vp;

template <class T>
class MeshRenderSystem : public System<TransformComponent, MeshComponent<T>>
{
public:
    MeshRenderSystem(Renderer* renderer) : m_renderer(renderer) {}
    
    void RenderShadow()
    {
        // First Pass:
        // Create depth buffer in light space
        m_renderer->UseDepthBuffer(1);
        m_renderer->ClearDepth();
        
        for (size_t i=0; i<SystemBase::m_entities.size(); ++i)
        {
            Entity* entity = SystemBase::m_entities[i];
            auto mc = entity->GetComponent<MeshComponent<T>>();
            m_renderer->transforms.model = entity->GetComponent<TransformComponent>()->GetTransform()
                                            * Scale(mc->scale);
            m_renderer->transforms.mvp = light_vp * m_renderer->transforms.model;
            mc->mesh.Draw(shadersDepth);
        }

     
    }
    void Render()
    {
        // Second Pass:
        // Render the scene and use previous depth buffer for shadow mapping
        m_renderer->UseDepthBuffer(0);
        m_renderer->ClearColorAndDepth();

        for (size_t i=0; i<SystemBase::m_entities.size(); ++i)
        {
            Entity* entity = SystemBase::m_entities[i];
            auto mc = entity->GetComponent<MeshComponent<T>>();
            m_renderer->transforms.model = entity->GetComponent<TransformComponent>()->GetTransform()
                                            * Scale(mc->scale);
            m_renderer->transforms.mvp = vp * m_renderer->transforms.model;
            m_renderer->transforms.bias_light_mvp = bias_matrix * light_vp * m_renderer->transforms.model;
            mc->material.DrawMesh(mc->mesh);
        }
    }

private:
    Renderer* m_renderer;
};
