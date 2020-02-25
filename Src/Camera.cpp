
#include "../Include/Camera.h"

void Camera::initProjection(const float fov, const float aspectRatio, const float nearPlane, const float farPlane)
{
    mProjMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    mProjMatrix[1][1] *= -1.0f;

    //const float sizeX = 110.0f;
    //const float sizeY = sizeX * 9.0f / 16.0f;
    //mProjMatrix = glm::ortho(-sizeX, sizeX, sizeY, -sizeY, 1.0f, 400.0f);
}

const float fov = 45.0f;
const float aspectRatio = 16.0f / 9.0f;
const float nearPlane = 0.1f;
const float farPlane = 400.0f;

Camera::Camera() : mWorldMatrix{glm::mat4(1.0f)}
{
    initProjection(fov, aspectRatio, nearPlane, farPlane);
}

void Camera::moveHorizontal(float distance)
{
    mPosition += glm::vec3{mWorldMatrix[0][0], mWorldMatrix[0][1], mWorldMatrix[0][2]} * distance;
}

void Camera::moveVertical(float distance)
{
    mPosition += glm::vec3{mWorldMatrix[1][0], mWorldMatrix[1][1], mWorldMatrix[1][2]} * distance;
}

void Camera::moveDiagonal(float distance)
{
    mPosition += glm::vec3{mWorldMatrix[2][0], mWorldMatrix[2][1], mWorldMatrix[2][2]} * distance;
}

void Camera::Yaw(float angle)
{
    mYaw += angle;
}

void Camera::Pitch(float angle)
{
    mPitch += angle;
}

void Camera::update()
{
    auto t = glm::translate(glm::mat4{1.0f}, mPosition);
    auto yaw = glm::rotate(glm::mat4{1.0f}, mYaw, glm::vec3{0.0f, 1.0f, 0.0f});
    auto pitch = glm::rotate(glm::mat4{1.0f}, mPitch, glm::vec3{1.0f, 0.0f, 0.0f});
    mWorldMatrix = t * yaw * pitch;
}





