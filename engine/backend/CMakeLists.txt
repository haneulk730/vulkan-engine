cmake_minimum_required(VERSION 3.8)
project(engine LANGUAGES C CXX)

set(TARGET backend)
set(PUBLIC_HDR_DIR include)

set(PUBLIC_HDRS include/backend/Platform.h)

set(SRCS src/Driver.cpp src/Platform.cpp src/PlatformFactory.cpp)

set(PRIVATE_HDRS include/private/backend/Driver.h
                 include/private/backend/PlatformFactory.h src/DriverBase.h)

list(
  APPEND
  SRCS
  include/backend/platforms/VulkanPlatform.h
  src/vulkan/platform/VulkanPlatform.cpp
  src/vulkan/utils/Helper.h
  src/vulkan/VulkanContext.cpp
  src/vulkan/VulkanContext.h
  src/vulkan/VulkanDriver.cpp
  src/vulkan/VulkanDriver.h)

include_directories(${PUBLIC_HDR_DIR})
include_directories(src)

add_library(${TARGET} STATIC ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

set_target_properties(${TARGET} PROPERTIES FOLDER Engine)

target_link_libraries(${TARGET} PUBLIC volk absl::log)
