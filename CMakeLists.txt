cmake_minimum_required(VERSION 3.5.0)
project(ft_vox VERSION 1.0.0)
cmake_policy(SET CMP0072 NEW)

find_package(OpenGL REQUIRED)

add_executable(ft_vox
	srcs/main.cpp
	srcs/utils.cpp
	includes/classes/Shader.cpp
	includes/classes/WindowsHandler.cpp
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

target_compile_options(ft_vox PRIVATE -Wall -Wextra -O3 -g -fPIE)
