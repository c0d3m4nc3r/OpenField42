#include "core/camera.h"

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
) : position(position),
    rotation(rotation),
    aspect_ratio(aspect_ratio),
    near_plane(near_plane),
    far_plane(far_plane),
    field_of_view(field_of_view) {}

void Camera::move(const glm::vec3& delta_pos)
{
    position += delta_pos;
    view_dirty = true;
}

void Camera::rotate(const glm::vec3& delta_rot)
{
    rotation += delta_rot;
    view_dirty = true;
}

const glm::vec3& Camera::getPosition() const { return position; }
const glm::vec3& Camera::getRotation() const { return rotation; }

const glm::vec3& Camera::getForward()
{
    if (view_dirty) update();
    return forward;
}

const glm::vec3& Camera::getRight()
{
    if (view_dirty) update();
    return right;
}

const glm::vec3& Camera::getUp()
{
    if (view_dirty) update();
    return up;
}

const glm::mat4& Camera::getViewMat()
{
    if (view_dirty) update();
    return view_mat;
}

const glm::mat4& Camera::getProjMat()
{
    if (proj_dirty) update();
    return proj_mat;
}

const Frustum& Camera::getFrustum()
{
    if (view_dirty || proj_dirty)
        update();
    return frustum;
}

float Camera::getAspectRatio() const { return aspect_ratio; }
float Camera::getNearPlane() const { return near_plane; }
float Camera::getFarPlane() const { return far_plane; }
float Camera::getFieldOfView() const { return field_of_view; }

void Camera::setPosition(const glm::vec3& position)
{
    this->position = position; view_dirty = true;
}

void Camera::setRotation(const glm::vec3& rotation)
{
    this->rotation = rotation; view_dirty = true;
}

void Camera::setAspectRatio(float aspect_ratio)
{
    this->aspect_ratio = aspect_ratio; proj_dirty = true;
}

void Camera::setNearPlane(float near_plane)
{
    this->near_plane = near_plane; proj_dirty = true;
}

void Camera::setFarPlane(float far_plane)
{
    this->far_plane = far_plane; proj_dirty = true;
}

void Camera::setFieldOfView(float field_of_view)
{
    this->field_of_view = field_of_view; proj_dirty = true;
}

void Camera::update()
{
    if (view_dirty)
    {
        glm::mat4 rotation_matrix = glm::eulerAngleYXZ(
            glm::radians(-rotation.y),
            glm::radians(rotation.x),
            glm::radians(rotation.z)
        );

        forward = glm::normalize(glm::vec3(rotation_matrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
        right   = glm::normalize(glm::vec3(rotation_matrix * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
        up      = glm::normalize(glm::vec3(rotation_matrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));

        view_mat = glm::lookAt(position, position + forward, up);
        view_dirty = false;
    }

    if (proj_dirty)
    {
        proj_mat = glm::perspective(
            glm::radians(field_of_view),
            aspect_ratio,
            near_plane,
            far_plane
        );

        proj_dirty = false;
    }

    frustum.extract(proj_mat * view_mat);
}
