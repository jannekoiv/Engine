#include "includes.h"

Engine::Engine(const int width, const int height)
    : _context{width, height},
      _camera{
          glm::radians(45.0f),
          static_cast<float>(width) / static_cast<float>(height),
          0.1f,
          100.0f}
{
    std::cout << "Engine initialized\n";
}

Engine::~Engine()
{
}
