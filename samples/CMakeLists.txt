cmake_minimum_required(VERSION 3.8)
project(samples LANGUAGES C CXX)

function(add_demo NAME)
  add_executable(${NAME} ${NAME}.cpp)
  target_link_libraries(${NAME} PRIVATE Vulkan::Vulkan glm glfw)
  set_target_properties(${NAME} PROPERTIES FOLDER Samples)
endfunction()

add_demo(main)
