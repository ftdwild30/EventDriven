file(GLOB_RECURSE TEST_FILES ${ROOT}/test/*.cpp)


message(STATUS "this test file: ${TEST_FILES}")

add_executable(test ${TEST_FILES})
add_dependencies(test event_driven)
target_link_libraries(test event_driven pthread)