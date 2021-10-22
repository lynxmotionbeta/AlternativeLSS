

set(INCLUDE_FILES
		LssParser.h
		LssRequest.h
		LssCommand.h
		RequestBuffer.h
		LssBus.h
		LssSynthesize.h
		LssChannel.h
		DeviceArray.h
		DeviceIndex.h
		platform/LssChannelDriver.h
		platform/posix/LssPosixChannel.h
)
set(SOURCE_FILES
		LssParser.cpp
		LssRequest.cpp
		LssCommand.cpp
		RequestBuffer.cpp
        LssBus.cpp
		LssChannel.cpp
		DeviceIndex.cpp
		platform/LssChannelDriver.cpp
		platform/posix/LssPosixChannel.cpp
        LssSynthesize.cpp)


# we first add our intermediate object library
# which will them be linked into an archive and a shared library
#add_library(sigint-objlib OBJECT ${SOURCE_FILES} $<TARGET_OBJECTS:sigint-matrix> $<TARGET_OBJECTS:sigint-streams> $<TARGET_OBJECTS:sigint-numerical>)
add_library(lss-bus-objlib OBJECT ${INCLUDE_FILES} ${SOURCE_FILES})

# depend on bunch of C++11 and C++14 features to have C++14 enabled by default
target_compile_features(lss-bus-objlib
		PUBLIC
		cxx_alignas
		cxx_alignof
		cxx_attributes
		cxx_auto_type
		cxx_constexpr
		cxx_defaulted_functions
		cxx_deleted_functions
		cxx_final
		cxx_lambdas
		cxx_noexcept
		cxx_override
		cxx_range_for
		cxx_rvalue_references
		cxx_static_assert
		cxx_strong_enums
		cxx_trailing_return_types
		cxx_unicode_literals
		cxx_user_literals
		cxx_variable_templates
		cxx_variadic_macros
)

# set compilation properties on the object library
set_property(TARGET lss-bus-objlib PROPERTY POSITION_INDEPENDENT_CODE 1)
# target_compile_features(lss-bus-objlib PRIVATE cxx_std_14)
#set_property(TARGET lss-bus-objlib PROPERTY C_STANDARD 99)
#target_include_directories(lss-bus-objlib )

# now link the object lib into static archive and shared library versions
set(LSS_BUS_TARGET_OBJECTS $<TARGET_OBJECTS:lss-bus-objlib>)
add_library(lss-bus-static STATIC ${LSS_BUS_TARGET_OBJECTS})
add_library(lss-bus-shared SHARED ${LSS_BUS_TARGET_OBJECTS})

#SET_TARGET_PROPERTIES(lss-bus-static PROPERTIES OUTPUT_NAME lss CLEAN_DIRECT_OUTPUT 1)
#SET_TARGET_PROPERTIES(lss-bus-shared PROPERTIES OUTPUT_NAME lss CLEAN_DIRECT_OUTPUT 1)
#target_link_libraries(lss-bus-shared ...)

# store location of sigint variables in the cache for other projects to reference
set(LSS_BUS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "path to LSS bus library includes")
set(LSS_BUS_LIBRARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "path to LSS bus library binaries")
#set(LSS_LINK_LIBRARIES ${Boost_LIBRARIES} ${JSON-C_LIBRARY} CACHE INTERNAL "")

# attach version number to SO shared file
#set_target_properties(sigint-shared PROPERTIES
#        VERSION ${sigintSDK_VERSION_STRING}
#        SOVERSION ${sigintSDK_VERSION_MAJOR})


## Install headers
install(FILES ${INCLUDE_FILES}
	DESTINATION include/${PROJECT_NAME}
)
#install(DIRECTORY platform
#	DESTINATION include/${PROJECT_NAME}
#)

install(TARGETS lss-bus-static lss-bus-shared
        EXPORT export_${PROJECT_NAME}
        RUNTIME DESTINATION "bin"
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        INCLUDES DESTINATION "include"
        )

