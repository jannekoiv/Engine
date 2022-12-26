cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)
project (Engine)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#find_program(iwyu_path NAMES iwy FATAL_ERROR)
#if(NOT iwyu_path)
#    message(FATAL_ERROR ${iwyu_path})
#endif()

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLSLang REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

#file(GLOB SOURCES "src/*.cpp" "src/miniz.c")
#set_source_files_properties("src/miniz.c" PROPERTIES LANGUAGE CXX )
file(GLOB SOURCES "src/*.cpp" "src/miniz.c")
set_source_files_properties("src/miniz.c" PROPERTIES LANGUAGE CXX )

add_library(engine SHARED ${SOURCES})
target_include_directories(engine PRIVATE "/home/jak/koodaus/stuntcar/engine/include" "/home/jak/koodaus/stuntcar/imgui/include")
target_link_libraries(engine miniz OpenFBX glfw imgui Vulkan::Vulkan SPIRV SPIRV-Tools SPIRV-Tools-opt glslang pthread MachineIndependent OSDependent GenericCodeGen OGLCompiler generator)
#target_link_libraries(engine glfw Vulkan::Vulkan SPIRV glslang pthread MachineIndependent OSDependent GenericCodeGen OGLCompiler)
target_compile_features(engine PRIVATE cxx_std_17) 

#set_property(TARGET test PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
target_precompile_headers(engine PRIVATE "include/ext_includes.h")
