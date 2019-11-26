
# we first add our intermediate object library
# which will them be linked into an archive and a shared library
#add_library(sigint-objlib OBJECT ${SOURCE_FILES} $<TARGET_OBJECTS:sigint-matrix> $<TARGET_OBJECTS:sigint-streams> $<TARGET_OBJECTS:sigint-numerical>)
add_executable(lss-compliantjoint CompliantJoints.cpp CompliantJoint.h CompliantJoint.cpp)

# currently no dependencies
add_dependencies(lss-compliantjoint lss-static)
include_directories(${LSS_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/eigen)
link_directories(${LSS_LIBRARY_DIR})
target_link_libraries(lss-compliantjoint lss-static pthread)

# set compilation properties on the object library
set_property(TARGET lss-objlib PROPERTY POSITION_INDEPENDENT_CODE 1)
#set_property(TARGET lss-objlib PROPERTY C_STANDARD 99)
#target_include_directories(lss-objlib )

install(TARGETS lss-compliantjoint
        RUNTIME DESTINATION "bin"
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib")