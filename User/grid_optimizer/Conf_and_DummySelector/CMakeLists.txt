add_library(UserConf SHARED UserConf.cxx)
target_link_libraries(UserConf ${ROOT_LIBRARIES} Conf)

add_library(UserSelector SHARED UserSelector.cxx)
target_link_libraries(UserSelector ${ROOT_LIBRARIES} ${CORE_LIBRARIES} ${CONTAINER_LIBRARIES}
                                    ${PHYSICS_LIBRARIES} ${CONF_LIBRARIES} ${SELECTOR_LIBRARIES})
list(APPEND USER_LIBRARIES UserSelector)

set(USER_LIBRARIES ${USER_LIBRARIES} PARENT_SCOPE)