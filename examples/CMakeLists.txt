
function(lss_bus_example example_name)
    add_executable(${example_name} ${example_name}.cpp)

    # currently no dependencies
    target_link_libraries(${example_name} PUBLIC lss-bus-static)
    add_dependencies(${example_name} lss-bus-static)
    target_include_directories(${example_name} PUBLIC ${LSS_BUS_INCLUDE_DIR})

    # set compilation properties on the object library
    set_property(TARGET ${example_name} PROPERTY POSITION_INDEPENDENT_CODE 1)

    if(HAS_LIBFTDI)
        target_link_libraries(${example_name} ftdi)
    endif()

    if(VALGRIND)
        add_dependencies(memcheck ${example_name}-memcheck)
        add_custom_target(${example_name}-memcheck
                COMMAND "${VALGRIND}" --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes $<TARGET_FILE:${example_name}>)
    endif()

    install(TARGETS ${example_name}
            RUNTIME DESTINATION "bin"
            LIBRARY DESTINATION "lib"
            ARCHIVE DESTINATION "lib")
endfunction()

add_subdirectory(BusScan)
add_subdirectory(ReadStateLoop)
add_subdirectory(StateAndCommand)

if(WITH_COMPLIANCE)
    #add_subdirectory(CompliantJoint)
endif()

#add_subdirectory(MultiRead)
#add_subdirectory(Humanoid)
