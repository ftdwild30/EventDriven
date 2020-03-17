file(GLOB_RECURSE SRC_FILES ${ROOT}/src/*.cpp)
include_directories(${ROOT}/src)

message(STATUS "this test file: ${SRC_FILES}")

add_library(event_driven STATIC ${SRC_FILES})
set_target_properties(event_driven PROPERTIES OUTPUT_NAME "event_driven")
set_target_properties(event_driven PROPERTIES CLEAN_DIRECT_OUTPUT 1)