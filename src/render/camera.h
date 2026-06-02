#pragma once

#include "math/frustum.h"

#include <glm/vec3.hpp>

class Camera
{
public:

    Camera(
        const glm::vec3& position = glm::vec3(0.0f),
        const glm::vec3& rotation = glm::vec3(0.0f),
        float aspect_ratio = 16.0f/9.0f,
        float near_plane = 0.5f,
        float far_plane = 1000.0f,
        float field_of_view = 75.0f
    );

    void move(const glm::vec3& delta_pos);
    void rotate(const glm::vec3& delta_rot);

    const glm::vec3& getPosition() const;
    const glm::vec3& getRotation() const;

    const glm::vec3& getForward() const;
    const glm::vec3& getRight() const;
    const glm::vec3& getUp() const;

    const glm::mat4& getViewMat() const;
    const glm::mat4& getProjMat() const;

    const Frustum& getFrustum() const;

    float getAspectRatio() const;
    float getNearPlane() const;
    float getFarPlane() const;
    float getFieldOfView() const;

    void setPosition(const glm::vec3& position);
    void setRotation(const glm::vec3& rotation);

    void setAspectRatio(float aspect_ratio);
    void setNearPlane(float near_plane);
    void setFarPlane(float far_plane);
    void setFieldOfView(float field_of_view);

private:
    
    glm::vec3 _position, _rotation;
    
    float _aspect_ratio;
    float _near_plane;
    float _far_plane;
    float _field_of_view;
    
    mutable glm::vec3 _forward, _right, _up;
    mutable glm::mat4 _view_mat, _proj_mat;
    mutable bool _view_dirty, _proj_dirty;
    
    mutable Frustum _frustum;

    void update() const;
};
