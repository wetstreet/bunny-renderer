# Copyright (C) 2018 Tomasz Gałaj

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(BunnyRenderer)

# Add .lib files
link_directories(${CMAKE_SOURCE_DIR}/lib)

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/*.c
	${CMAKE_SOURCE_DIR}/src/*.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/src/*.h
	${CMAKE_SOURCE_DIR}/src/*.hpp)

# Configure assets header file
configure_file(src/helpers/RootDir.h.in src/helpers/RootDir.h)
include_directories(${CMAKE_BINARY_DIR}/src)
	
# Define the executable
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

# We need a CMAKE_DIR with some code to find external dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

#######################################
# LOOK for the packages that we need! #
#######################################

# Define the include DIRs
include_directories(
	"${CMAKE_SOURCE_DIR}/src"
	"${CMAKE_SOURCE_DIR}/include"
	"${CMAKE_SOURCE_DIR}/include/imgui"
	"${CMAKE_SOURCE_DIR}/include/glad"
	"${CMAKE_SOURCE_DIR}/include/glm"
	"${CMAKE_SOURCE_DIR}/include/ImGuizmo"
)

# OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# GLM
find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")

# GLFW
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

# ASSIMP
find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")

# STB_IMAGE
add_library(STB_IMAGE "thirdparty/stb_image.cpp")

# STB_IMAGE_WRITE
add_library(STB_IMAGE_WRITE "thirdparty/stb_image_write.cpp")

# tinyexr
add_library(TINYEXR "thirdparty/tinyexr.cpp" "thirdparty/miniz.c")

# GLAD
add_library(GLAD "thirdparty/glad.c")

# imgui
file(GLOB IMGUI_SOURCES "thirdparty/imgui/*.cpp")
add_library(IMGUI ${IMGUI_SOURCES})

# ImGuizmo
add_library(IMGUIZMO "thirdparty/ImGuizmo.cpp")

find_library(COMCTL32 Comctl32.lib)
find_library(PROPSYS Propsys.lib)
find_library(SHLWAPI Shlwapi.lib)
find_library(D3D11 d3d11.lib)
find_library(D3D11COMPILER d3dcompiler.lib)
find_library(DXGI dxgi.lib)

# Put all libraries into a variable
set(LIBS ${GLFW3_LIBRARY} ${OPENGL_LIBRARY} IMGUI GLAD ${CMAKE_DL_LIBS} ${ASSIMP_LIBRARY} STB_IMAGE STB_IMAGE_WRITE TINYEXR IMGUIZMO ${COMCTL32} ${PROPSYS} ${SHLWAPI} ${D3D11} ${D3D11COMPILER} ${DXGI})

# Define the link libraries
target_link_libraries(${PROJECT_NAME} ${LIBS})

if(WIN32)
	# Copy dlls
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${PROJECT_SOURCE_DIR}/dlls"
		$<TARGET_FILE_DIR:${PROJECT_NAME}>)

	# Copy resources
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${PROJECT_SOURCE_DIR}/res"
		"${CMAKE_BINARY_DIR}/res")
		
	# Copy imgui config
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"${PROJECT_SOURCE_DIR}/imgui.ini"
		"${CMAKE_BINARY_DIR}/imgui.ini")
endif()