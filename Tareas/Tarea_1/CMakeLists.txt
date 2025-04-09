cmake_minimum_required(VERSION 4)
set(CMAKE_CXX_STANDARD 17)

project(matrices)

add_subdirectory(src)
if (BUILD_TESTING)
  add_subdirectory(test)
endif()

