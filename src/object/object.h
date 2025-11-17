#pragma once

#include <memory>
#include <vector>
#include <string>

#include <glm/glm.hpp>

enum class ObjectType
{
    Unknown,
    ANDCompositeObjective,
    ActiveKitPart,
    AnimatedBundle,
    AreaObject,
    BFSoldier,
    Bundle,
    Camera,
    ClusterProjectile,
    ControlPoint,
    DestroyTargetObjective,
    EffectBundle,
    Emitter,
    Engine,
    EntryPoint,
    FireArms,
    FireArmsBundle,
    Flag,
    FlagBase,
    FloatingBundle,
    FreeCamera,
    HandFireArms,
    Hook,
    Kit,
    KitPart,
    LandingGear,
    LensFlare,
    LodObject,
    MusicPlayer,
    ObjectSpawner,
    Obstacle,
    Particle,
    PlayerControlObject,
    Projectile,
    RotationalBundle,
    SeatObject,
    SonarObject,
    SpawnPoint,
    Spring,
    SpriteParticle,
    SupplyDepot,
    TimerObjective,
    Turbulence,
    Winch,
    Wing,
    WireLink,
    SimpleObject
};

std::string objectTypeToString(ObjectType type);
ObjectType objectTypeFromString(const std::string& str);

class Geometry;
class Shader;
class ObjectTemplate;
struct Object
{
    ObjectType type = ObjectType::Unknown;
    Object* parent = nullptr;
    glm::vec3 continous_rot_speed = glm::vec3(0.0f);

    static inline std::vector<std::unique_ptr<Object>> registry;
    static inline Object* current = nullptr;

    Object(
        const glm::vec3& position = glm::vec3(0.0f),
        const glm::vec3& rotation = glm::vec3(0.0f),
        const glm::vec3& scale = glm::vec3(1.0f)
    );

    static Object* create(const ObjectTemplate* tmpl);

    void draw();
    void update(float dt);

    void move(const glm::vec3& delta_pos);
    void rotate(const glm::vec3& delta_rot);

    const glm::vec3& getPosition() const;
    const glm::vec3& getRotation() const;
    const glm::vec3& getScale() const;

    const glm::mat4& getModelMatrix();

    void setPosition(const glm::vec3& position);
    void setRotation(const glm::vec3& rotation);
    void setScale(const glm::vec3& scale);

    void setDirty(bool dirty = true);

private:

    glm::vec3 position, rotation, scale;
    glm::mat4 model_mat;
    bool dirty = true;

    Geometry* geometry = nullptr;

    std::vector<Object*> children;
};