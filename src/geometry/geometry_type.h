#pragma once

#include "utils/string_utils.h"

enum class GeometryType : unsigned char
{
    Unknown,
    AnimatedMesh,
    SkeletonCollisionMesh,
    StandardMesh,
    TreeMesh,
    PatchTerrain,

    // Custom
    SkyMesh,
    WaterMesh
};

inline std::string geometryTypeToString(GeometryType type)
{
    switch (type)
    {
    case GeometryType::AnimatedMesh: return "AnimatedMesh";
    case GeometryType::SkeletonCollisionMesh: return "SkeletonCollisionMesh";
    case GeometryType::StandardMesh: return "StandardMesh";
    case GeometryType::TreeMesh: return "TreeMesh";
    case GeometryType::PatchTerrain: return "PatchTerrain";
    case GeometryType::Unknown:
    default: return "Unknown";
    }
}

inline GeometryType geometryTypeFromString(const std::string& str)
{
    static const std::unordered_map<std::string, GeometryType> lut = {
        {"animatedmesh", GeometryType::AnimatedMesh},
        {"skeletoncollisionmesh", GeometryType::SkeletonCollisionMesh},
        {"standardmesh", GeometryType::StandardMesh},
        {"treemesh", GeometryType::TreeMesh},
        {"patchterrain", GeometryType::PatchTerrain}
    };

    auto it = lut.find(StringUtils::lowercase(str));
    if (it != lut.end())
        return it->second;

    return GeometryType::Unknown;
}
