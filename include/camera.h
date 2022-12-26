#pragma once

#include "ext_includes.h"

class Camera {
public:
    Camera(
        const float fov,
        const float aspect_ratio,
        const float near_plane,
        const float far_plane);

    const glm::mat4& world_matrix();

    glm::mat4 view_matrix();

    const glm::mat4& proj_matrix();

    void move_horizontal(float distance);
    void move_vertical(float distance);
    void move_diagonal(float distance);

    void yaw(float angle);
    void pitch(float angle);

private:
    void update();

    glm::vec3 _position;
    float _yaw;
    float _pitch;
    glm::mat4 _world_matrix;
    glm::mat4 _proj_matrix;
    bool _world_matrix_outdated;
};
