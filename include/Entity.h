#pragma once
#include <Component.h>

class Entity
{
public:
    ~Entity()
    {
        for (auto it=m_components.begin(); it!=m_components.end(); ++it)
            delete it->second;
        m_components.clear();
    }
    
    template<class ComponentType, class... Args>
    ComponentType* AddComponent(Args... args)
    {
        auto comp = m_components[(uint32_t)ComponentType::GetType()] = new ComponentType(args...);
        return (ComponentType*)comp;
    }
    
    template<class ComponentType>
    ComponentType* GetComponent()
    {
        return (ComponentType*)m_components[(uint32_t)ComponentType::GetType()];
    }

    template<class ComponentType>
    bool HasComponent()
    {
        return m_components.find((uint32_t)ComponentType::GetType()) != m_components.end();
    }

private:
    std::unordered_map<uint32_t, ComponentBase*> m_components;
};
