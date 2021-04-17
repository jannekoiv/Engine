#pragma once

#include "base.h"

class Camera {
public:
    Camera();

    void init_projection(
        const float fov, const float aspect_ratio, const float near_plane, const float far_plane);

    const glm::mat4 world_matrix()
    {
        return _world_matrix;
    }

    void set_world_matrix(glm::mat4 world_matrix)
    {
        _world_matrix = world_matrix;
    }

    const glm::mat4 view_matrix()
    {
        return glm::inverse(_world_matrix);
    }

    const glm::mat4& proj_matrix() const
    {
        return _proj_matrix;
    }

    void move_horizontal(float distance);
    void move_vertical(float distance);
    void move_diagonal(float distance);

    void yaw(float angle);
    void pitch(float angle);

    void update();

private:
    glm::vec3 _position;
    float _yaw;
    float _pitch;
    glm::mat4 _world_matrix;
    glm::mat4 _proj_matrix;
};
