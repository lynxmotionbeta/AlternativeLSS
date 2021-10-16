add_subdirectory(BusScan)
add_subdirectory(ReadStateLoop)
add_subdirectory(StateAndCommand)

if(WITH_COMPLIANCE)
    #add_subdirectory(CompliantJoint)
endif()

#add_subdirectory(MultiRead)
#add_subdirectory(Humanoid)
