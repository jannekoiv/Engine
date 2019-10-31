cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(Engine CXX)

include_directories(SYSTEM "/home/jak/eclipse-workspace/Engine/Include")

include(CheckCXXCompilerFlag)

function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
        unset(flag_supported CACHE)
    endif()
endfunction()

#enable_cxx_compiler_flag_if_supported("-fno-elide-constructors")
enable_cxx_compiler_flag_if_supported("-Wall")
enable_cxx_compiler_flag_if_supported("-Wextra")
enable_cxx_compiler_flag_if_supported("-pedantic")


set(CMAKE_CXX_STANDARD 11)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.2 REQUIRED)


add_library(Engine "")

target_sources(
	Engine
	PRIVATE
	Src/Base.cpp
	Src/Buffer.cpp
	Src/DescriptorPool.cpp
	Src/DescriptorSetLayout.cpp
	Src/Device.cpp
	Src/Engine.cpp
	Src/Image.cpp
	Src/Model.cpp
	Src/Pipeline.cpp
	Src/Renderer.cpp
	Src/RenderPass.cpp
	Src/Sampler.cpp
	Src/Semaphore.cpp
	Src/ShaderModule.cpp
	Src/SwapChain.cpp)

target_link_libraries(
	Engine
	${Vulkan_LIBRARIES}
	glfw)
	
add_executable(EngineTest "")

target_sources(
	EngineTest
	PRIVATE
	Test/TestMain.cpp
	Test/DummyTest.cpp)

add_executable(TestApp "")

target_sources(
	TestApp
	PRIVATE
	Src/TestApp.cpp)

target_link_libraries(
	TestApp
	Engine)
		


