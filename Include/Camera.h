#pragma once

#include "Base.h"

class Camera {
public:
    Camera();

    void initProjection(
        const float fov, const float aspectRatio, const float nearPlane, const float farPlane);

    const glm::mat4 worldMatrix()
    {
        return mWorldMatrix;
    }

    void setWorldMatrix(glm::mat4 worldMatrix)
    {
        mWorldMatrix = worldMatrix;
    }

    const glm::mat4 viewMatrix()
    {
        return glm::inverse(mWorldMatrix);
    }

    const glm::mat4& projMatrix() const
    {
        return mProjMatrix;
    }

    void moveHorizontal(float distance);
    void moveVertical(float distance);
    void moveDiagonal(float distance);

    void Yaw(float angle);
    void Pitch(float angle);

    void update();

private:
    glm::vec3 mPosition;
    float mYaw;
    float mPitch;
    glm::mat4 mWorldMatrix;
    glm::mat4 mProjMatrix;
};
