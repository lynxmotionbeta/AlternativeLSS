
set(SOURCE_FILES
        catch.hpp
        lss-tests.cpp
        packet.cpp
        commands.cpp
)

# we first add our intermediate object library
# which will them be linked into an archive and a shared library
#add_library(sigint-objlib OBJECT ${SOURCE_FILES} $<TARGET_OBJECTS:sigint-matrix> $<TARGET_OBJECTS:sigint-streams> $<TARGET_OBJECTS:sigint-numerical>)
add_executable(lss-tests ${SOURCE_FILES})
include_directories(${LSS_INCLUDE_DIR})
add_dependencies(lss-tests lss-static)
target_link_libraries(lss-tests lss-static pthread)

# set compilation properties on the object library
set_property(TARGET lss-tests PROPERTY POSITION_INDEPENDENT_CODE 1)
# target_compile_features(lss-objlib PRIVATE cxx_std_14)
#set_property(TARGET lss-objlib PROPERTY C_STANDARD 99)

#SET_TARGET_PROPERTIES(lss-static PROPERTIES OUTPUT_NAME lss CLEAN_DIRECT_OUTPUT 1)
#SET_TARGET_PROPERTIES(lss-shared PROPERTIES OUTPUT_NAME lss CLEAN_DIRECT_OUTPUT 1)

install(TARGETS lss-tests
        RUNTIME DESTINATION "bin"
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib")
