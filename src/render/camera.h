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

    const glm::vec3& getForward();
    const glm::vec3& getRight();
    const glm::vec3& getUp();

    const glm::mat4& getViewMat();
    const glm::mat4& getProjMat();

    const Frustum& getFrustum();

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
    
    glm::vec3 position, rotation;
    glm::vec3 forward, right, up;
    glm::mat4 view_mat, proj_mat;

    Frustum frustum;

    float aspect_ratio;
    float near_plane;
    float far_plane;
    float field_of_view;

    bool view_dirty;
    bool proj_dirty;

    void update();
};
