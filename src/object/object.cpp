#include "object/object.h"

#include "utils/string_utils.h"

#include <glm/gtc/matrix_transform.hpp>

std::string objectTypeToString(ObjectType type)
{
    switch (type)
    {
    case ObjectType::ANDCompositeObjective: return "ANDCompositeObjective";
    case ObjectType::ActiveKitPart: return "ActiveKitPart";
    case ObjectType::AnimatedBundle: return "AnimatedBundle";
    case ObjectType::AreaObject: return "AreaObject";
    case ObjectType::BFSoldier: return "BFSoldier";
    case ObjectType::Bundle: return "Bundle";
    case ObjectType::Camera: return "Camera";
    case ObjectType::ClusterProjectile: return "ClusterProjectile";
    case ObjectType::ControlPoint: return "ControlPoint";
    case ObjectType::DestroyTargetObjective: return "DestroyTargetObjective";
    case ObjectType::EffectBundle: return "EffectBundle";
    case ObjectType::Emitter: return "Emitter";
    case ObjectType::Engine: return "Engine";
    case ObjectType::EntryPoint: return "EntryPoint";
    case ObjectType::FireArms: return "FireArms";
    case ObjectType::Flag: return "Flag";
    case ObjectType::FlagBase: return "FlagBase";
    case ObjectType::FloatingBundle: return "FloatingBundle";
    case ObjectType::FreeCamera: return "FreeCamera";
    case ObjectType::HandFireArms: return "HandFireArms";
    case ObjectType::Hook: return "Hook";
    case ObjectType::Kit: return "Kit";
    case ObjectType::KitPart: return "KitPart";
    case ObjectType::LandingGear: return "LandingGear";
    case ObjectType::LensFlare: return "LensFlare";
    case ObjectType::LodObject: return "LodObject";
    case ObjectType::MusicPlayer: return "MusicPlayer";
    case ObjectType::ObjectSpawner: return "ObjectSpawner";
    case ObjectType::Obstacle: return "Obstacle";
    case ObjectType::Particle: return "Particle";
    case ObjectType::PlayerControlObject: return "PlayerControlObject";
    case ObjectType::Projectile: return "Projectile";
    case ObjectType::RotationalBundle: return "RotationalBundle";
    case ObjectType::SeatObject: return "SeatObject";
    case ObjectType::SonarObject: return "SonarObject";
    case ObjectType::SpawnPoint: return "SpawnPoint";
    case ObjectType::Spring: return "Spring";
    case ObjectType::SpriteParticle: return "SpriteParticle";
    case ObjectType::SupplyDepot: return "SupplyDepot";
    case ObjectType::TimerObjective: return "TimerObjective";
    case ObjectType::Turbulence: return "Turbulence";
    case ObjectType::Winch: return "Winch";
    case ObjectType::Wing: return "Wing";
    case ObjectType::WireLink: return "WireLink";
    case ObjectType::SimpleObject: return "SimpleObject";
    default: return "Unknown";
    }
}

ObjectType objectTypeFromString(const std::string& str)
{   
    static const std::unordered_map<std::string, ObjectType> lut = {
        {"andcompositeobjective", ObjectType::ANDCompositeObjective},
        {"activekitpart", ObjectType::ActiveKitPart},
        {"animatedbundle", ObjectType::AnimatedBundle},
        {"areaobject", ObjectType::AreaObject},
        {"bfsoldier", ObjectType::BFSoldier},
        {"bundle", ObjectType::Bundle},
        {"camera", ObjectType::Camera},
        {"clusterprojectile", ObjectType::ClusterProjectile},
        {"controlpoint", ObjectType::ControlPoint},
        {"destroytargetobjective", ObjectType::DestroyTargetObjective},
        {"effectbundle", ObjectType::EffectBundle},
        {"emitter", ObjectType::Emitter},
        {"engine", ObjectType::Engine},
        {"entrypoint", ObjectType::EntryPoint},
        {"firearms", ObjectType::FireArms},
        {"flag", ObjectType::Flag},
        {"flagbase", ObjectType::FlagBase},
        {"floatingbundle", ObjectType::FloatingBundle},
        {"freecamera", ObjectType::FreeCamera},
        {"handfirearms", ObjectType::HandFireArms},
        {"hook", ObjectType::Hook},
        {"kit", ObjectType::Kit},
        {"kitpart", ObjectType::KitPart},
        {"landinggear", ObjectType::LandingGear},
        {"lensflare", ObjectType::LensFlare},
        {"lodobject", ObjectType::LodObject},
        {"musicplayer", ObjectType::MusicPlayer},
        {"objectspawner", ObjectType::ObjectSpawner},
        {"obstacle", ObjectType::Obstacle},
        {"particle", ObjectType::Particle},
        {"playercontrolobject", ObjectType::PlayerControlObject},
        {"projectile", ObjectType::Projectile},
        {"rotationalbundle", ObjectType::RotationalBundle},
        {"seatobject", ObjectType::SeatObject},
        {"sonarobject", ObjectType::SonarObject},
        {"spawnpoint", ObjectType::SpawnPoint},
        {"spring", ObjectType::Spring},
        {"spriteparticle", ObjectType::SpriteParticle},
        {"supplydepot", ObjectType::SupplyDepot},
        {"timerobjective", ObjectType::TimerObjective},
        {"turbulence", ObjectType::Turbulence},
        {"winch", ObjectType::Winch},
        {"wing", ObjectType::Wing},
        {"wirelink", ObjectType::WireLink},
        {"simpleobject", ObjectType::SimpleObject}
    };

    auto it = lut.find(StringUtils::lowercase(str));
    if (it != lut.end())
        return it->second;

    return ObjectType::Unknown;
}

Object::Object(
    const glm::vec3& position,
    const glm::vec3& rotation,
    const glm::vec3& scale
) : _position(position), _rotation(rotation), _scale(scale), _model_mat(1.0f) {}

void Object::update(float dt)
{
    rotate(continous_rot_speed * dt);
}

void Object::move(const glm::vec3& delta_pos)
{
    setPosition(_position + delta_pos);
}

void Object::rotate(const glm::vec3& delta_rot)
{
    setRotation(_rotation + delta_rot);
}

void Object::addChild(Object* object)
{
    _children.push_back(object);
}

const glm::mat4& Object::getModelMatrix()
{
    if (_dirty)
    {
        _model_mat = glm::mat4(1.0f);
        _model_mat = glm::translate(_model_mat, _position);
        _model_mat = glm::rotate(_model_mat, glm::radians(_rotation.x), glm::vec3(0,1,0));
        _model_mat = glm::rotate(_model_mat, glm::radians(_rotation.y), glm::vec3(1,0,0));
        _model_mat = glm::rotate(_model_mat, glm::radians(_rotation.z), glm::vec3(0,0,1));
        _model_mat = glm::scale(_model_mat, _scale);

        if (parent)
            _model_mat = parent->getModelMatrix() * _model_mat;

        _dirty = false;
    }

    return _model_mat;
}

void Object::setDirty(bool dirty)
{
    _dirty = dirty;

    for (auto child : _children)
        if (child) child->setDirty(dirty);
}
