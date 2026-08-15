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

    void update(float dt);
    void render();

    Object* createObject(const ObjectTemplate* tmpl);

    Sky& getSky() { return _sky; }
    Terrain& getTerrain() { return _terrain; }
    Water& getWater() { return _water; }

private:

    Sky _sky;
    Terrain _terrain;
    Water _water;

    std::vector<std::unique_ptr<Object>> _objects;
};
