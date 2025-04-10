add_library(matrix STATIC Matrix.cpp)
target_include_directories(matrix PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(main main.cpp)
target_link_libraries(main matrix)

