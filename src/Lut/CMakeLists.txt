file(GLOB LUT_SOURCES  *.cxx)

add_library(Lut SHARED ${LUT_SOURCES})
target_link_libraries(Lut Units Settings ReactionFragment ${ROOT_LIBRARIES})
list(APPEND LUT_LIBRARIES Lut)

set(LUT_LIBRARIES ${LUT_LIBRARIES} PARENT_SCOPE)

# Install the built libraries
install(TARGETS ${LUT_LIBRARIES}
    LIBRARY DESTINATION lib       # For shared libraries (.so/.dylib)
    ARCHIVE DESTINATION lib       # For static libraries (.a)
    RUNTIME DESTINATION bin)      # For executables (if any)

# Install header files (excluding the LinkDef.h files)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION include/Lut
        FILES_MATCHING PATTERN "*.h")
