cmake_minimum_required(VERSION 3.10.0)
project(ft_vox VERSION 1.0.0)
cmake_policy(SET CMP0072 NEW)

find_package(OpenGL REQUIRED)

add_executable(ft_vox
	# Sources (UI)
	srcs/main.cpp
	srcs/rendering.cpp
	srcs/events.cpp
	srcs/utils.cpp
	srcs/flags.cpp

	# Framework
	framework/classes/Shader.cpp
	framework/classes/Window.cpp
	framework/classes/Camera.cpp
	framework/classes/Profiler.cpp
	framework/classes/Noise.cpp
	framework/classes/SkyBox.cpp
	framework/classes/PMapBufferGL.cpp
	framework/classes/BufferGL.cpp

	# Includes
	includes/classes/VoxelSystem.cpp
	includes/classes/Chunks/AChunk.cpp
	includes/classes/Chunks/ChunkImpl.cpp
	includes/classes/Chunks/ChunkHandler.cpp

	# Dependencies
	dependencies/glad/glad.c
	dependencies/glfw/libglfw3.a
	dependencies/glm/libglm.a
	dependencies/stb_image/stb_def.c
)

target_include_directories(ft_vox PRIVATE
	${CMAKE_SOURCE_DIR}/dependencies
	${CMAKE_SOURCE_DIR}/framework
	${CMAKE_SOURCE_DIR}/framework/classes
	${CMAKE_SOURCE_DIR}/includes
	${CMAKE_SOURCE_DIR}/includes/classes
	${CMAKE_SOURCE_DIR}/includes/classes/Chunks
)

target_link_libraries(ft_vox PRIVATE
	OpenGL::GL
	glfw
)

target_compile_options(ft_vox PRIVATE -Wall -Wextra -O3 -g -fPIE)
