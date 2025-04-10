add_subdirectory(extern)
add_executable(tests test.cpp)
target_link_libraries(tests PRIVATE GTest::gtest_main matrix)
target_include_directories(tests PRIVATE ${PROJECT_SOURCE_DIR}/include)

include(GoogleTest)
gtest_discover_tests(tests)

