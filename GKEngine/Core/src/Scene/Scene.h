#pragma once
#include "../ECS/Entity.h"
#include "../../include/GKEngineAPI.h"
#include <vector>
#include <memory>
#include <string>

namespace GK {

class Scene {
public:
    GK_RESULT load(const char* path);
    GK_RESULT save(const char* path) const;

    int     entityCount() const          { return (int)m_entities.size(); }
    Entity* entityAt(int i)              { return m_entities[i].get(); }
    Entity* createEntity(const char* name);
    void    destroyEntity(Entity* e);
    Entity* findById(int id);

    const std::string& name() const { return m_name; }

private:
    std::string                          m_name;
    std::vector<std::unique_ptr<Entity>> m_entities;
    int                                  m_nextId = 1;

    void addDefaultEntities3D();
};

} // namespace GK
