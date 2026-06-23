#pragma once

#include "object/object_type.h"

#include <vector>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class Geometry;
class Shader;
class Terrain;
struct ObjectTemplate;
struct Object
{
    ObjectType type = ObjectType::Unknown;
    Object* parent = nullptr;
    glm::vec3 continous_rot_speed = glm::vec3(0.0f);

    static inline Object* current = nullptr;

    Object(
        const glm::vec3& position = glm::vec3(0.0f),
        const glm::vec3& rotation = glm::vec3(0.0f),
        const glm::vec3& scale = glm::vec3(1.0f)
    );

    void update(float dt);

    void move(const glm::vec3& delta_pos);
    void rotate(const glm::vec3& delta_rot);

    void addChild(Object* object);

    const glm::vec3& getPosition() const { return _position; }
    const glm::vec3& getRotation() const { return _rotation; }
    const glm::vec3& getScale() const { return _scale; }

    const glm::mat4& getModelMatrix();

    Geometry* getGeometry() { return _geometry; }

    void setPosition(const glm::vec3& position) { _position = position; setDirty(); }
    void setRotation(const glm::vec3& rotation) { _rotation = rotation; setDirty(); }
    void setScale(const glm::vec3& scale) { _scale = scale; setDirty(); }

    void setGeometry(Geometry* geometry) { _geometry = geometry; }

    void setDirty(bool dirty = true);

private:

    glm::vec3 _position, _rotation, _scale;
    glm::mat4 _model_mat;
    bool _dirty = true;

    Geometry* _geometry = nullptr;

    std::vector<Object*> _children;
};
