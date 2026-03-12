#pragma once
#include <string>
#include <glm/glm.hpp>

namespace GK {

// ─── Component structs (POD-friendly, 직접 저장) ──────────────
struct Vec3 { float x=0, y=0, z=0; };
struct RGB  { float r=1, g=1, b=1; };

struct TransformData {
    Vec3 position;
    Vec3 rotation;
    Vec3 scale = {1,1,1};
};

struct CameraData {
    float fov       = 60.f;
    float nearPlane = 0.1f;
    float farPlane  = 1000.f;
    RGB   clearColor = {0.1f, 0.1f, 0.16f};
    bool  isOrthographic = false;
};

enum class LightType { Directional=0, Point=1, Spot=2 };

struct LightData {
    LightType type      = LightType::Directional;
    RGB       color     = {1,1,1};
    float     intensity = 1.f;
    bool      castShadows = true;
    float     range     = 10.f;
    float     spotAngle = 30.f;
};

struct MeshData {
    std::string meshName = "BuiltIn/Cube";
    std::string material = "Default";
    bool castShadows    = true;
    bool receiveShadows = true;
};

// ─── Entity ───────────────────────────────────────────────────
class Entity {
public:
    explicit Entity(int id, const std::string& name)
        : m_id(id), m_name(name) {}

    int               id()     const { return m_id; }
    const std::string& name()  const { return m_name; }
    void setName(const std::string& n) { m_name = n; }
    bool isActive() const { return m_active; }
    void setActive(bool a) { m_active = a; }

    // Component access (always present — zero-initialized)
    TransformData& transform() { return m_transform; }
    const TransformData& transform() const { return m_transform; }

    bool        hasCamera() const { return m_hasCamera; }
    CameraData& camera()          { m_hasCamera = true; return m_camera; }

    bool       hasLight() const { return m_hasLight; }
    LightData& light()         { m_hasLight = true; return m_light; }

    bool      hasMesh() const { return m_hasMesh; }
    MeshData& mesh()          { m_hasMesh = true; return m_mesh; }

    // Icon string for Hierarchy panel
    const char* icon() const {
        if (m_hasCamera) return "📷";
        if (m_hasLight)  return "💡";
        if (m_hasMesh)   return "⬛";
        return "○";
    }

private:
    int         m_id;
    std::string m_name;
    bool        m_active    = true;

    TransformData m_transform;
    CameraData    m_camera;
    LightData     m_light;
    MeshData      m_mesh;

    bool m_hasCamera = false;
    bool m_hasLight  = false;
    bool m_hasMesh   = false;
};

} // namespace GK
