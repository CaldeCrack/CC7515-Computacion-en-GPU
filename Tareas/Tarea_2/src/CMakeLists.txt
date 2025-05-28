find_package(OpenCL REQUIRED)

file(GLOB_RECURSE KERNELS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cl)
foreach(KERNEL IN LISTS KERNELS)
    file(COPY ${KERNEL} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
endforeach()

add_executable(serial serial.cpp)
add_executable(cuda tpb.cu cuda.cu)
add_executable(opencl tpb.cpp)

target_link_libraries(opencl PRIVATE ${OpenCL_LIBRARIES})

