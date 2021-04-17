
#include "camera.h"

void Camera::init_projection(const float fov, const float aspect_ratio, const float near_plane, const float far_plane)
{
    _proj_matrix = glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
    _proj_matrix[1][1] *= -1.0f;

    //const float sizeX = 110.0f;
    //const float sizeY = sizeX * 9.0f / 16.0f;
    //mProjMatrix = glm::ortho(-sizeX, sizeX, sizeY, -sizeY, 1.0f, 400.0f);
}

const float fov = 45.0f;
const float aspect_ratio = 16.0f / 9.0f;
const float near_plane = 0.1f;
const float far_plane = 400.0f;

Camera::Camera() : _world_matrix{glm::mat4(1.0f)}
{
    init_projection(fov, aspect_ratio, near_plane, far_plane);
}

void Camera::move_horizontal(float distance)
{
    _position += glm::vec3{_world_matrix[0][0], _world_matrix[0][1], _world_matrix[0][2]} * distance;
}

void Camera::move_vertical(float distance)
{
    _position += glm::vec3{_world_matrix[1][0], _world_matrix[1][1], _world_matrix[1][2]} * distance;
}

void Camera::move_diagonal(float distance)
{
    _position += glm::vec3{_world_matrix[2][0], _world_matrix[2][1], _world_matrix[2][2]} * distance;
}

void Camera::yaw(float angle)
{
    _yaw += angle;
}

void Camera::pitch(float angle)
{
    _pitch += angle;
}

void Camera::update()
{
    auto t = glm::translate(glm::mat4{1.0f}, _position);
    auto yaw = glm::rotate(glm::mat4{1.0f}, _yaw, glm::vec3{0.0f, 1.0f, 0.0f});
    auto pitch = glm::rotate(glm::mat4{1.0f}, _pitch, glm::vec3{1.0f, 0.0f, 0.0f});
    _world_matrix = t * yaw * pitch;
}





