cmake_minimum_required (VERSION 2.8)
project(3dGameEngine)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules/")

find_package(GLFW REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)

set(SRCS
  src/main.cpp
  src/Primitives.cpp
  src/AssetManager.cpp
  src/draw.cpp
)
set(HDRS
  include/ShaderProgram.hpp
  src/Primitives.hpp
  src/AssetManager.hpp
  src/draw.hpp
)

include_directories(
  ${GLEW_INCLUDE_DIRS}
  ${GLFW_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
  include
  include/fx-gltf/include
  include/json/include
)
link_libraries(
  ${GLEW_LIBRARIES}
  ${GLFW_LIBRARIES}
  ${GLM_LIBRARIES}
)

file(
  COPY ./dist
  DESTINATION ${CMAKE_BINARY_DIR}
)
add_executable(3dGameEngine ${SRCS} ${HDRS})

set_property(TARGET 3dGameEngine PROPERTY CXX_STANDARD_REQUIRED true)
set_property(TARGET 3dGameEngine PROPERTY CXX_STANDARD 17)

if(MSVC)
  target_compile_options(3dGameEngine PRIVATE "/W4")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(3dGameEngine PRIVATE "-Wall")
  target_compile_options(3dGameEngine PRIVATE "-Wextra")
endif()
