#pragma once

class Object;
struct ObjectTemplate;
class ObjectManager
{
public:

    void registerCmds() const;

    Object* createObject(const ObjectTemplate* tmpl);

    void updateObjects(float dt);
    void renderObjects();

private:

    std::vector<std::unique_ptr<Object>> _objects;
    
};
