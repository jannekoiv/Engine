#pragma once

#include "camera.h"
#include "context.h"
#include "ext_includes.h"

class Engine {
public:
    Engine(const Engine&) = delete;

    Engine(Engine&&) = delete;

    Engine(const int width, const int height);

    ~Engine();

    Engine& operator=(const Engine&) = delete;

    Engine& operator=(Engine&&) = delete;

    Camera& camera()
    {
        return _camera;
    }

    Context& context()
    {
        return _context;
    }

private:
    Context _context;

public:
    Camera _camera;
};
