#pragma once

class GeometryManager;

struct ObjectTemplate;
struct Object;

class Sky;
class Terrain;
class Water;

class Renderer;

class World
{
public:

    World(GeometryManager& geometry_mgr);
    ~World();

    void init();
    void shutdown();
    void update(float dt);

    Object* createObject(const ObjectTemplate* tmpl);
    void renderObjects(Renderer& renderer) const;
    void renderTerrain(Renderer& renderer) const;

    Sky& getSky() { return *_sky; }
    Terrain& getTerrain() { return *_terrain; }
    Water& getWater() { return *_water; }

private:

    GeometryManager& _geometry_mgr;

    std::unique_ptr<Sky> _sky;
    std::unique_ptr<Terrain> _terrain;
    std::unique_ptr<Water> _water;

    std::vector<std::unique_ptr<Object>> _objects;
};
