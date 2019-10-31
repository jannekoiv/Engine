
#pragma once

#include "../Include/Base.h"

class Device;

class Semaphore {
public:
    Semaphore(Device& device);

    ~Semaphore();

    operator vk::Semaphore() const
    {
        return mSemaphore;
    }

private:
    Device& mDevice;
    vk::Semaphore mSemaphore;
};
