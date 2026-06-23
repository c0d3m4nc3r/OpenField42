#pragma once

#include "utils/string_utils.h"

enum class ObjectType : unsigned char
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

inline std::string objectTypeToString(ObjectType type)
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

inline ObjectType objectTypeFromString(const std::string& str)
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
