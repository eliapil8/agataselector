set(BUILD_DICTIONARIES FALSE)

if(BUILD_DICTIONARIES)
    message("Building physics dictionaries")
    ROOT_GENERATE_DICTIONARY(G__Fitter Fitter.h LINKDEF FitterLinkDef.h)
    add_library(Fitter SHARED Fitter.cxx G__Fitter.cxx)
    target_link_libraries(Fitter ${ROOT_LIBRARIES})
    list(APPEND OPTIMIZER_LIBRARIES Fitter)
else()
    add_library(Fitter SHARED Fitter.cxx)
    target_link_libraries(Fitter ${ROOT_LIBRARIES})
    list(APPEND OPTIMIZER_LIBRARIES Fitter)
endif()


add_library(Optimizer SHARED Optimizer.cxx)
target_link_libraries(Optimizer ${ROOT_LIBRARIES} ${CONF_LIBRARIES} ${LUT_LIBRARIES}
                        ${CORE_LIBRARIES} ${OPTIMIZER_LIBRARIES} App)
list(APPEND OPTIMIZER_LIBRARIES Optimizer)

set(OPTIMIZER_LIBRARIES ${OPTIMIZER_LIBRARIES} PARENT_SCOPE)

# Install the built libraries
install(TARGETS ${OPTIMIZER_LIBRARIES}
    LIBRARY DESTINATION lib       # For shared libraries (.so/.dylib)
    ARCHIVE DESTINATION lib       # For static libraries (.a)
    RUNTIME DESTINATION bin)      # For executables (if any)

# Install header files (excluding the LinkDef.h files)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION include/Optimizer
        FILES_MATCHING PATTERN "*.h")
