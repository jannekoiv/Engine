
#include "includes.h"

glm::mat4 world_matrix(const glm::vec3& position, float yaw, float pitch)
{
    auto translate = glm::translate(glm::mat4{1.0f}, position);
    auto yaw_matrix = glm::rotate(glm::mat4{1.0f}, yaw, glm::vec3{0.0f, 1.0f, 0.0f});
    auto pitch_matrix = glm::rotate(glm::mat4{1.0f}, pitch, glm::vec3{1.0f, 0.0f, 0.0f});
    return translate * yaw_matrix * pitch_matrix;
}

glm::mat4 proj_matrix(
    const float fov,
    const float aspect_ratio,
    const float near_plane,
    const float far_plane)
{
    auto matrix = glm::perspective(fov, aspect_ratio, near_plane, far_plane);
    matrix[1][1] *= -1.0f;
    return matrix;
}

Camera::Camera(
    const float fov,
    const float aspect_ratio,
    const float near_plane,
    const float far_plane)
    : _position{0.0f, 0.0f, 0.0f},
      _yaw{0.0f},
      _pitch{0.0f},
      _world_matrix{1.0f},
      _proj_matrix{::proj_matrix(fov, aspect_ratio, near_plane, far_plane)},
      _world_matrix_outdated{false}
{
}

void Camera::update()
{
    if (_world_matrix_outdated) {
        _world_matrix = ::world_matrix(_position, _yaw, _pitch);
        _world_matrix_outdated = false;
    }
}

const glm::mat4& Camera::world_matrix()
{
    update();
    return _world_matrix;
}

glm::mat4 Camera::view_matrix()
{
    update();
    return glm::inverse(_world_matrix);
}

const glm::mat4& Camera::proj_matrix()
{
    return _proj_matrix;
}

void Camera::move_horizontal(float distance)
{
    _position +=
        glm::vec3{_world_matrix[0][0], _world_matrix[0][1], _world_matrix[0][2]} *
        distance;
    _world_matrix_outdated = true;
}

void Camera::move_vertical(float distance)
{
    _position +=
        glm::vec3{_world_matrix[1][0], _world_matrix[1][1], _world_matrix[1][2]} *
        distance;
    _world_matrix_outdated = true;
}

void Camera::move_diagonal(float distance)
{
    _position +=
        glm::vec3{_world_matrix[2][0], _world_matrix[2][1], _world_matrix[2][2]} *
        distance;
    _world_matrix_outdated = true;
}

void Camera::yaw(float angle)
{
    _yaw += angle;
    _world_matrix_outdated = true;
}

void Camera::pitch(float angle)
{
    _pitch += angle;
    _world_matrix_outdated = true;
}
