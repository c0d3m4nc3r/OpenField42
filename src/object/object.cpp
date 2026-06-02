#include "object/object.h"
#include "object/template.h"
#include "utils/log.h"
#include "render/renderer.h"
#include "render/shader.h"
#include "geometry/template.h"
#include "geometry/standard_mesh.h"
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

Object* Object::create(const ObjectTemplate* tmpl)
{
    if (!tmpl)
    {
        LOG_ERROR("Object::create: template is NULL!");
        return nullptr;
    }
    
    auto obj = std::make_unique<Object>();
    obj->type = tmpl->type;
    obj->continous_rot_speed = tmpl->continous_rot_speed;

    Object* raw = obj.get();

    for (const auto& child : tmpl->children)
    {
        auto child_tmpl = ObjectTemplate::get(child.tmpl_name);
        if (!child_tmpl)
        {
            LOG_ERROR("Object::create: Failed to create child: Object template with name '%s' not found!", child.tmpl_name.c_str());
            continue;
        }

        auto child_ptr = Object::create(child_tmpl);
        if (!child_ptr)
        {
            LOG_ERROR("Object::create: Failed to create child object!");
            continue;
        }

        child_ptr->setPosition(child.position);
        child_ptr->setRotation(child.rotation);
        child_ptr->parent = raw;
        obj->_children.push_back(child_ptr);
    }

    if (tmpl->geometry.empty())
    {
        registry.push_back(std::move(obj));

        current = raw;
        return raw;
    }

    const GeometryTemplate* geom_tmpl = GeometryTemplate::get(tmpl->geometry);
    if (!geom_tmpl)
    {
        LOG_ERROR("Object::create: Geometry template with name '%s' not found!", tmpl->geometry.c_str());
        return nullptr;
    }

    // Added this to prevent spam in logs
    // Should be removed when the TreeMesh type will be supported
    if (geom_tmpl->type == GeometryType::TreeMesh)
    {
        registry.push_back(std::move(obj));

        current = raw;
        return raw;
    }

    obj->_geometry = Geometry::create(geom_tmpl);
    if (!obj->_geometry)
    {
        LOG_ERROR("Object::create: Failed to load geometry!");
        return nullptr;
    }

    registry.push_back(std::move(obj));

    current = raw;
    return raw;
}

void Object::draw()
{
    if (!_geometry) return;
    g_Renderer.submit(_geometry, getModelMatrix());
}

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

const glm::vec3& Object::getPosition() const { return _position; }
const glm::vec3& Object::getRotation() const { return _rotation; }
const glm::vec3& Object::getScale() const { return _scale; }

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

void Object::setPosition(const glm::vec3& position)
{
    this->_position = position; setDirty();
}

void Object::setRotation(const glm::vec3& rotation)
{
    this->_rotation = rotation; setDirty();
}

void Object::setScale(const glm::vec3& scale)
{
    this->_scale = scale; setDirty();
}

void Object::setDirty(bool dirty)
{
    this->_dirty = dirty;

    for (auto child : _children)
        if (child) child->setDirty(dirty);
}
