add_subdirectory(extern)
add_executable(tests test.cpp)
target_link_libraries(tests PRIVATE GTest::gtest_main matrix)

include(GoogleTest)
gtest_discover_tests(tests)

