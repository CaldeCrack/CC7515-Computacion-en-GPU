add_library(Matrix STATIC Matrix.cpp)
target_include_directories(Matrix PUBLIC ${PROJECT_SOURCE_DIR}/include)

# add_executable(matrices main.cpp)
target_link_libraries(matrices Matrix)
