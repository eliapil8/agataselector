message("Building tools dictionaries")
#add_library(GateSelector SHARED GateSelector.cxx)
#target_link_libraries(GateSelector ${ROOT_LIBRARIES} Units)
#list(APPEND TOOLS_LIBRARIES GateSelector)

add_subdirectory(EdECalculator)
list(APPEND TOOLS_LIBRARIES ${EDE_CALCULATOR_LIBRARIES})

add_library(IDLine SHARED IDLine.cxx)
target_link_libraries(IDLine ${ROOT_LIBRARIES} ${TOOLS_LIBRARIES})
list(APPEND TOOLS_LIBRARIES IDLine)

set(TOOLS_LIBRARIES ${TOOLS_LIBRARIES} PARENT_SCOPE)

# Install the built libraries
install(TARGETS ${TOOLS_LIBRARIES}
    LIBRARY DESTINATION lib       # For shared libraries (.so/.dylib)
    ARCHIVE DESTINATION lib       # For static libraries (.a)
    RUNTIME DESTINATION bin)      # For executables (if any)

# Install header files (excluding the LinkDef.h files)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION include/Tools
        FILES_MATCHING PATTERN "*.h")
