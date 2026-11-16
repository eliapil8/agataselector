file(GLOB CONF_SOURCES  *.cxx)
list(REMOVE_ITEM CONF_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Conf.cxx)
list(REMOVE_ITEM CONF_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ConfManager.cxx)

add_library(Conf SHARED Conf.cxx)
target_link_libraries(Conf Units Settings ${ROOT_LIBRARIES})
list(APPEND CONF_LIBRARIES Conf)

add_library(ConfDetectors SHARED ${CONF_SOURCES})
target_link_libraries(ConfDetectors Units Settings Lut ReactionFragment Conf ${ROOT_LIBRARIES})
list(APPEND CONF_LIBRARIES ConfDetectors)

add_library(ConfManager SHARED ConfManager.cxx)
target_link_libraries(  ConfManager Units
                        ReactionFragment  Settings
                        ${CONF_LIBRARIES} ${LUT_LIBRARIES}
                        UserConf Conf
                        ${ROOT_LIBRARIES})
list(APPEND CONF_LIBRARIES ConfManager)

set(CONF_LIBRARIES ${CONF_LIBRARIES} PARENT_SCOPE)

# Install the built libraries
install(TARGETS ${CONF_LIBRARIES}
    LIBRARY DESTINATION lib       # For shared libraries (.so/.dylib)
    ARCHIVE DESTINATION lib       # For static libraries (.a)
    RUNTIME DESTINATION bin)      # For executables (if any)

# Install header files (excluding the LinkDef.h files)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION include/Conf
        FILES_MATCHING PATTERN "*.h")
