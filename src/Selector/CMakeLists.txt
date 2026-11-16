file(GLOB SELECTOR_SOURCES  *.cxx CoincidenceAnalysis/*.cxx DetectorAnalysis/*.cxx)

add_library(AgataSelector SHARED ${SELECTOR_SOURCES})
target_link_libraries(AgataSelector ${ROOT_LIBRARIES} ${CORE_LIBRARIES} ${CONTAINER_LIBRARIES}
        ${PHYSICS_LIBRARIES} ${CONF_LIBRARIES} ${TOOLS_LIBRARIES})
list(APPEND SELECTOR_LIBRARIES AgataSelector)

set(SELECTOR_LIBRARIES ${SELECTOR_LIBRARIES} PARENT_SCOPE)

# Install the built libraries
install(TARGETS ${SELECTOR_LIBRARIES}
    LIBRARY DESTINATION lib       # For shared libraries (.so/.dylib)
    ARCHIVE DESTINATION lib       # For static libraries (.a)
    RUNTIME DESTINATION bin)      # For executables (if any)

# Install header files (excluding the LinkDef.h files)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION include/Selector
        FILES_MATCHING PATTERN "*.h")
