#include "object/object.h"

#include "utils/string_utils.h"

#include <glm/gtc/matrix_transform.hpp>

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
