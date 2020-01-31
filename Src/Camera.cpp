
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
//const float nearPlane = 0.1f;
//const float farPlane = 500.0f;
const float nearPlane = 1.0f;
const float farPlane = 400.0f;

Camera::Camera() : mWorldMatrix{glm::mat4(1.0f)}
{
    initProjection(fov, aspectRatio, nearPlane, farPlane);
    mWorldMatrix = glm::translate(mWorldMatrix, glm::vec3(0.0f, 0.0f, 250.0f));
}

void Camera::moveHorizontal(float distance)
{
    mWorldMatrix = glm::translate(mWorldMatrix, glm::vec3(distance, 0.0f, 0.0f));
}

void Camera::moveVertical(float distance)
{
    mWorldMatrix = glm::translate(mWorldMatrix, glm::vec3(0.0f, distance, 0.0f));
}

void Camera::moveDiagonal(float distance)
{
    mWorldMatrix = glm::translate(mWorldMatrix, glm::vec3(0.0f, 0.0f, distance));
}

void Camera::Yaw(float angle)
{
    mWorldMatrix = glm::rotate(mWorldMatrix, angle, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::Pitch(float angle)
{
    mWorldMatrix = glm::rotate(mWorldMatrix, angle, glm::vec3(1.0f, 0.0f, 0.0f));
}
