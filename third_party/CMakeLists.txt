cmake_minimum_required(VERSION 3.8)
project(third_party LANGUAGES C CXX)

# glm
add_library(glm INTERFACE)
set(GLM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glm)
target_sources(glm INTERFACE ${GLM_DIR}/glm/glm.hpp)
target_include_directories(glm SYSTEM INTERFACE ${GLM_DIR})

target_compile_definitions(
  glm INTERFACE GLM_FORCE_SWIZZLE GLM_FORCE_RADIANS GLM_FORCE_CTOR_INIT
                GLM_ENABLE_EXPERIMENTAL)
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  target_compile_definitions(glm INTERFACE GLM_FORCE_CXX14)
endif()

# GLFW
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_INSTALL OFF)
option(BUILD_SHARED_LIBS ON)

add_subdirectory(glfw)

if(TARGET update_mappings)
  set_target_properties(update_mappings PROPERTIES FOLDER "ThirdParty")
endif()
set_property(TARGET glfw PROPERTY FOLDER "ThirdParty")

# Abseil
add_subdirectory(abseil-cpp)

# volk
if(WIN32)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()

add_subdirectory(volk)

set_property(TARGET volk PROPERTY FOLDER "ThirdParty")
