#pragma once

#include "object/object.h"
#include "world/sky.h"
#include "world/terrain.h"
#include "world/water.h"

struct ObjectTemplate;

class GeometryManager;
class Renderer;

class World
{
public:

    void registerCmds() const;

    void render();

    Sky& getSky() { return _sky; }
    Terrain& getTerrain() { return _terrain; }
    Water& getWater() { return _water; }

private:

    Sky _sky;
    Terrain _terrain;
    Water _water;
};
