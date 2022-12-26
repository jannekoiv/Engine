#pragma once

//#define VULKAN_HPP_NO_SMART_HANDLE
//#define GLM_FORCE_ALIGNED
//#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GENERATOR_USE_GLM

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <stddef.h>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
