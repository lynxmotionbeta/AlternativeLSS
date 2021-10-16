
# we first add our intermediate object library
# which will them be linked into an archive and a shared library
#add_library(sigint-objlib OBJECT ${SOURCE_FILES} $<TARGET_OBJECTS:sigint-matrix> $<TARGET_OBJECTS:sigint-streams> $<TARGET_OBJECTS:sigint-numerical>)
add_executable(lss-read-state-loop ReadStateLoop.cpp)

# currently no dependencies
target_link_libraries(lss-read-state-loop PUBLIC lss-bus-static)
add_dependencies(lss-read-state-loop lss-bus-static)
target_include_directories(lss-read-state-loop PUBLIC ${LSS_BUS_INCLUDE_DIR})

#link_directories(${LSS_LIBRARY_DIR})

# set compilation properties on the object library
#set_property(TARGET lss-objlib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET lss-objlib PROPERTY C_STANDARD 99)
#target_include_directories(lss-objlib)

if(HAS_LIBFTDI)
    target_link_libraries(lss-read-state-loop ftdi)
endif()

if(VALGRIND)
    add_dependencies(memcheck memcheck-lss-read-state-loop)
    add_custom_target(memcheck-lss-read-state-loop
            COMMAND "${VALGRIND}" --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes $<TARGET_FILE:lss-read-state-loop>)
endif()

install(TARGETS lss-read-state-loop
        RUNTIME DESTINATION "bin"
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib")
