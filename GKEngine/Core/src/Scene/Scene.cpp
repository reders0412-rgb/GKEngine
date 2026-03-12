#include "Scene.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

namespace GK {

GK_RESULT Scene::load(const char* path) {
    if (!std::filesystem::exists(path)) {
        addDefaultEntities3D();
        return GK_OK;
    }
    std::ifstream f(path);
    auto j = nlohmann::json::parse(f, nullptr, false);
    if (j.is_discarded()) return GK_ERR_GENERIC;

    m_name = j.value("name", "Scene");

    for (auto& ej : j["entities"]) {
        int  id   = ej.value("id",   m_nextId);
        auto name = ej.value("name", "Entity");
        auto e = std::make_unique<Entity>(id, name);
        if (id >= m_nextId) m_nextId = id + 1;

        for (auto& comp : ej["components"]) {
            std::string type = comp.value("type","");
            if (type == "Transform") {
                auto& p = comp["position"]; auto& r = comp["rotation"]; auto& s = comp["scale"];
                e->transform().position = { p[0], p[1], p[2] };
                e->transform().rotation = { r[0], r[1], r[2] };
                e->transform().scale    = { s[0], s[1], s[2] };
            } else if (type == "Camera") {
                e->camera().fov       = comp.value("fov",  60.f);
                e->camera().nearPlane = comp.value("near", 0.1f);
                e->camera().farPlane  = comp.value("far",  1000.f);
            } else if (type == "Light") {
                e->light().intensity = comp.value("intensity", 1.f);
            } else if (type == "MeshRenderer") {
                e->mesh().meshName = comp.value("mesh",     "BuiltIn/Cube");
                e->mesh().material = comp.value("material", "Default");
            }
        }
        m_entities.push_back(std::move(e));
    }
    return GK_OK;
}

GK_RESULT Scene::save(const char* path) const {
    nlohmann::json j;
    j["version"] = "1.0";
    j["name"]    = m_name;
    j["entities"] = nlohmann::json::array();

    for (auto& e : m_entities) {
        nlohmann::json ej;
        ej["id"]   = e->id();
        ej["name"] = e->name();
        ej["components"] = nlohmann::json::array();

        auto& t = e->transform();
        ej["components"].push_back({
            {"type","Transform"},
            {"position",{t.position.x,t.position.y,t.position.z}},
            {"rotation",{t.rotation.x,t.rotation.y,t.rotation.z}},
            {"scale",   {t.scale.x,   t.scale.y,   t.scale.z   }}
        });
        if (e->hasCamera()) {
            auto& c = e->camera();
            ej["components"].push_back({
                {"type","Camera"},{"fov",c.fov},{"near",c.nearPlane},{"far",c.farPlane}
            });
        }
        if (e->hasLight()) {
            auto& l = e->light();
            ej["components"].push_back({
                {"type","Light"},{"intensity",l.intensity}
            });
        }
        if (e->hasMesh()) {
            auto& m = e->mesh();
            ej["components"].push_back({
                {"type","MeshRenderer"},{"mesh",m.meshName},{"material",m.material}
            });
        }
        j["entities"].push_back(ej);
    }
    std::ofstream(path) << j.dump(2);
    return GK_OK;
}

Entity* Scene::createEntity(const char* name) {
    auto e = std::make_unique<Entity>(m_nextId++, name);
    auto* ptr = e.get();
    m_entities.push_back(std::move(e));
    return ptr;
}

void Scene::destroyEntity(Entity* e) {
    m_entities.erase(
        std::remove_if(m_entities.begin(), m_entities.end(),
            [e](auto& u){ return u.get() == e; }),
        m_entities.end());
}

Entity* Scene::findById(int id) {
    for (auto& e : m_entities)
        if (e->id() == id) return e.get();
    return nullptr;
}

void Scene::addDefaultEntities3D() {
    m_name = "SampleScene";
    auto cam = std::make_unique<Entity>(m_nextId++, "Main Camera");
    cam->transform().position = {0,1,-10};
    cam->camera().fov = 60.f;
    m_entities.push_back(std::move(cam));

    auto light = std::make_unique<Entity>(m_nextId++, "Directional Light");
    light->transform().position = {0,10,0};
    light->transform().rotation = {50,-30,0};
    light->light().type = LightType::Directional;
    m_entities.push_back(std::move(light));

    auto cube = std::make_unique<Entity>(m_nextId++, "Cube");
    cube->transform().position = {0,0.5f,0};
    cube->mesh().meshName = "BuiltIn/Cube";
    m_entities.push_back(std::move(cube));

    auto plane = std::make_unique<Entity>(m_nextId++, "Plane");
    plane->transform().scale = {10,1,10};
    plane->mesh().meshName = "BuiltIn/Plane";
    m_entities.push_back(std::move(plane));
}

} // namespace GK
