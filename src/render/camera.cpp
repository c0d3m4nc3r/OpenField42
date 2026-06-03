#include "render/camera.h"

#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

Camera::Camera(
    const glm::vec3& position,
    const glm::vec3& rotation,
    float aspect_ratio,
    float near_plane,
    float far_plane,
    float field_of_view
) : _position(position),
    _rotation(rotation),
    _aspect_ratio(aspect_ratio),
    _near_plane(near_plane),
    _far_plane(far_plane),
    _field_of_view(field_of_view) {}

void Camera::move(const glm::vec3& delta_pos)
{
    _position += delta_pos;
    _view_dirty = true;
}

void Camera::rotate(const glm::vec3& delta_rot)
{
    _rotation += delta_rot;
    _view_dirty = true;
}

const glm::vec3& Camera::getPosition() const { return _position; }
const glm::vec3& Camera::getRotation() const { return _rotation; }

const glm::vec3& Camera::getForward() const
{
    if (_view_dirty) update();
    return _forward;
}

const glm::vec3& Camera::getRight() const
{
    if (_view_dirty) update();
    return _right;
}

const glm::vec3& Camera::getUp() const
{
    if (_view_dirty) update();
    return _up;
}

const glm::mat4& Camera::getViewMat() const
{
    if (_view_dirty) update();
    return _view_mat;
}

const glm::mat4& Camera::getProjMat() const
{
    if (_proj_dirty) update();
    return _proj_mat;
}

const Frustum& Camera::getFrustum() const
{
    if (_view_dirty || _proj_dirty) update();
    return _frustum;
}

float Camera::getAspectRatio() const { return _aspect_ratio; }
float Camera::getNearPlane() const { return _near_plane; }
float Camera::getFarPlane() const { return _far_plane; }
float Camera::getFieldOfView() const { return _field_of_view; }

void Camera::setPosition(const glm::vec3& position)
{
    this->_position = position; _view_dirty = true;
}

void Camera::setRotation(const glm::vec3& rotation)
{
    this->_rotation = rotation; _view_dirty = true;
}

void Camera::setAspectRatio(float aspect_ratio)
{
    this->_aspect_ratio = aspect_ratio; _proj_dirty = true;
}

void Camera::setNearPlane(float near_plane)
{
    this->_near_plane = near_plane; _proj_dirty = true;
}

void Camera::setFarPlane(float far_plane)
{
    this->_far_plane = far_plane; _proj_dirty = true;
}

void Camera::setFieldOfView(float field_of_view)
{
    this->_field_of_view = field_of_view; _proj_dirty = true;
}

void Camera::update() const
{
    if (_view_dirty)
    {
        glm::mat4 rotation_matrix = glm::eulerAngleYXZ(
            glm::radians(-_rotation.y),
            glm::radians(_rotation.x),
            glm::radians(_rotation.z)
        );

        _forward = glm::normalize(glm::vec3(rotation_matrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
        _right   = glm::normalize(glm::vec3(rotation_matrix * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
        _up      = glm::normalize(glm::vec3(rotation_matrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));

        _view_mat = glm::lookAt(_position, _position + _forward, _up);
        _view_dirty = false;
    }

    if (_proj_dirty)
    {
        _proj_mat = glm::perspective(
            glm::radians(_field_of_view),
            _aspect_ratio,
            _near_plane,
            _far_plane
        );

        _proj_dirty = false;
    }

    _frustum.extract(_proj_mat * _view_mat);
}
