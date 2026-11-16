add_library(Units SHARED Units.cxx)
list(APPEND CORE_LIBRARIES Units)

add_library(Settings SHARED Settings.cxx)
list(APPEND CORE_LIBRARIES Settings)

add_library(Print SHARED Print.cxx)
target_link_libraries(Print Settings ${ROOT_LIBRARIES})
list(APPEND CORE_LIBRARIES Print)

add_library(Worker SHARED Worker.cxx)
target_link_libraries(Worker ${ROOT_LIBRARIES} ${Boost_LIBRARIES} ${CONF_LIBRARIES} ${CORE_LIBRARIES} ${SELECTOR_LIBRARIES} ${CONTAINER_LIBRARIES} UserSelector)
list(APPEND CORE_LIBRARIES Worker)

add_library(Launcher SHARED Launcher.cxx)
target_link_libraries(Launcher ${ROOT_LIBRARIES} ${CONF_LIBRARIES})
list(APPEND CORE_LIBRARIES Launcher)

if(USE_MPI)
    add_library(LauncherDistributed SHARED LauncherDistributed.cxx)
    target_link_libraries(LauncherDistributed ${ROOT_LIBRARIES} ${Boost_LIBRARIES} ${CORE_LIBRARIES})
    list(APPEND CORE_LIBRARIES LauncherDistributed)
endif()

add_library(App SHARED App.cxx)
target_link_libraries(App ${ROOT_LIBRARIES} ${Boost_LIBRARIES} ${CORE_LIBRARIES} EdECalculatorOscar)
list(APPEND CORE_LIBRARIES App)

set(CORE_LIBRARIES ${CORE_LIBRARIES} PARENT_SCOPE)

# Install the built libraries
install(TARGETS ${CORE_LIBRARIES}
    LIBRARY DESTINATION lib       # For shared libraries (.so/.dylib)
    ARCHIVE DESTINATION lib       # For static libraries (.a)
    RUNTIME DESTINATION bin)      # For executables (if any)

# Install header files (excluding the LinkDef.h files)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION include/Core
        FILES_MATCHING PATTERN "*.h")
