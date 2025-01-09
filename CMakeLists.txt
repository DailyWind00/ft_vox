cmake_minimum_required(VERSION 3.5.0)
project(ft_vox VERSION 1.0.0)
cmake_policy(SET CMP0072 NEW)

find_package(OpenGL REQUIRED)

add_executable(ft_vox
	# sources (UI)
	srcs/main.cpp
	srcs/rendering.cpp
	srcs/events.cpp
	srcs/utils.cpp

	# Framework includes
	includes/classes/Shader.cpp
	includes/classes/Window.cpp
	includes/classes/Profiler.cpp

	# Dependencies
	dependencies/glad/glad.c
	dependencies/glfw/libglfw3.a
	dependencies/glm/libglm.a
	dependencies/stb_image/stb_def.c
)

target_include_directories(ft_vox PRIVATE
	${CMAKE_SOURCE_DIR}/dependencies
	${CMAKE_SOURCE_DIR}/includes
	${CMAKE_SOURCE_DIR}/includes/classes
)

target_link_libraries(ft_vox PRIVATE
	OpenGL::GL
	glfw
)

target_compile_options(ft_vox PRIVATE -Wall -Wextra -Werror -O3 -g -fPIE)
