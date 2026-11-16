set(BUILD_DICTIONARIES FALSE)

if(BUILD_DICTIONARIES)
    message("Building physics dictionaries")
    ROOT_GENERATE_DICTIONARY(G__ReactionFragment ReactionFragment.h LINKDEF ReactionFragmentLinkDef.h)
    add_library(ReactionFragment SHARED ReactionFragment.cxx G__ReactionFragment.cxx)
    target_link_libraries(ReactionFragment ${ROOT_LIBRARIES} Units )
    list(APPEND PHYSICS_LIBRARIES ReactionFragment)

    ROOT_GENERATE_DICTIONARY(G__ReactionReconstruction ReactionReconstruction.h LINKDEF ReactionReconstructionLinkDef.h)
    add_library(ReactionReconstruction SHARED ReactionReconstruction.cxx G__ReactionReconstruction.cxx)
    target_link_libraries(ReactionReconstruction ${ROOT_LIBRARIES}  ReactionFragment )
    list(APPEND PHYSICS_LIBRARIES ReactionReconstruction)

    ROOT_GENERATE_DICTIONARY(G__EnergyLoss EnergyLoss.h LINKDEF EnergyLossLinkDef.h)
    add_library(EnergyLoss SHARED EnergyLoss.cxx G__EnergyLoss.cxx)
    target_link_libraries(EnergyLoss ${ROOT_LIBRARIES} Units)
    list(APPEND PHYSICS_LIBRARIES EnergyLoss)
else()
    add_library(ReactionFragment SHARED ReactionFragment.cxx)
    target_link_libraries(ReactionFragment ${ROOT_LIBRARIES} Units )
    list(APPEND PHYSICS_LIBRARIES ReactionFragment)

    add_library(ReactionReconstruction SHARED ReactionReconstruction.cxx)
    target_link_libraries(ReactionReconstruction ${ROOT_LIBRARIES}  ReactionFragment )
    list(APPEND PHYSICS_LIBRARIES ReactionReconstruction)

    add_library(EnergyLoss SHARED EnergyLoss.cxx)
    target_link_libraries(EnergyLoss ${ROOT_LIBRARIES} Units)
    list(APPEND PHYSICS_LIBRARIES EnergyLoss)
endif()

set(PHYSICS_LIBRARIES ${PHYSICS_LIBRARIES} PARENT_SCOPE)

# Install the built libraries
install(TARGETS ${PHYSICS_LIBRARIES}
    LIBRARY DESTINATION lib       # For shared libraries (.so/.dylib)
    ARCHIVE DESTINATION lib       # For static libraries (.a)
    RUNTIME DESTINATION bin)      # For executables (if any)

# Install header files (excluding the LinkDef.h files)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION include/Physics
        FILES_MATCHING PATTERN "*.h")
