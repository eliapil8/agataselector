add_library(UserConf SHARED UserConf.cxx)
target_link_libraries(UserConf ${ROOT_LIBRARIES} Conf)
list(APPEND USER_LIBRARIES UserConf)

add_library(UserSelector SHARED UserSelector.cxx)
target_link_libraries(UserSelector ${ROOT_LIBRARIES} ${CORE_LIBRARIES} ${CONTAINER_LIBRARIES}
                                    ${PHYSICS_LIBRARIES} ${CONF_LIBRARIES} ${SELECTOR_LIBRARIES})
list(APPEND USER_LIBRARIES UserSelector)

set(USER_LIBRARIES ${USER_LIBRARIES} PARENT_SCOPE)

# Install the built libraries
install(TARGETS ${USER_LIBRARIES}
    LIBRARY DESTINATION lib       # For shared libraries (.so/.dylib)
    ARCHIVE DESTINATION lib       # For static libraries (.a)
    RUNTIME DESTINATION bin)      # For executables (if any)

# Install header files (excluding the LinkDef.h files)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION include/User
        FILES_MATCHING PATTERN "*.h")

