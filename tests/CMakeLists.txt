
cmake_minimum_required(VERSION 3.2)

set(SRC_LIST
        request-buffer.cpp parser.cpp synthesis.cpp indexes.cc)

message(STATUS "test sources ${SRC_LIST}")

add_definitions(-DUNIT_TESTING)

add_executable(lss-bus-tests ${SRC_LIST})
target_link_libraries(lss-bus-tests PUBLIC lss-bus-static)
target_link_libraries(lss-bus-tests PRIVATE Catch2::Catch2WithMain)

target_include_directories(lss-bus-tests PUBLIC ${LSS_BUS_INCLUDE_DIR})

include(CTest)
include(Catch)
catch_discover_tests(lss-bus-tests)

if(VALGRIND)
    message(STATUS "Enabled valgrind testing")
    add_dependencies(memcheck memcheck-lss-bus-tests)
    add_custom_target(memcheck-lss-bus-tests
            COMMAND "${VALGRIND}" --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes $<TARGET_FILE:lss-bus-tests>)
endif()


