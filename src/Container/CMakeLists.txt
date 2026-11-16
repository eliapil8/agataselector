file(GLOB CONTAINER_HEADERS  *.h)
list(FILTER CONTAINER_HEADERS EXCLUDE REGEX "LinkDef.h$")

foreach(HEADER ${CONTAINER_HEADERS})
   	get_filename_component(HEADER_NAME ${HEADER} NAME_WE)
        ROOT_GENERATE_DICTIONARY(G__${HEADER_NAME} ${HEADER} LINKDEF ${HEADER_NAME}LinkDef.h)
	if (RENAME_CONTAINER_LIBRARIES)
		add_library(${HEADER_NAME}Container SHARED ${HEADER} G__${HEADER_NAME}.cxx)
		message("Found container with name: ${HEADER_NAME}Container")
		if(HEADER_NAME MATCHES "Container")
			target_link_libraries(${HEADER_NAME}Container ${ROOT_LIBRARIES})
    	    elseif(HEADER_NAME MATCHES "Agata" OR HEADER_NAME MATCHES "Labr")
		        target_link_libraries(${HEADER_NAME}Container ${ROOT_LIBRARIES} ContainerContainer GammadetectorContainer)
		else()
		        target_link_libraries(${HEADER_NAME}Container ${ROOT_LIBRARIES} ContainerContainer)
		endif()
		list(APPEND CONTAINER_LIBRARIES ${HEADER_NAME}Container)
	else()
		add_library(${HEADER_NAME} SHARED ${HEADER} G__${HEADER_NAME}.cxx)
		message("Found container with name: ${HEADER_NAME}")
		if(HEADER_NAME MATCHES "Container")
			target_link_libraries(${HEADER_NAME} ${ROOT_LIBRARIES})
		elseif(HEADER_NAME MATCHES "Agata" OR HEADER_NAME MATCHES "Labr")
			target_link_libraries(${HEADER_NAME} ${ROOT_LIBRARIES} Container Gammadetector)
		else()
			target_link_libraries(${HEADER_NAME} ${ROOT_LIBRARIES} Container)
		endif()
		list(APPEND CONTAINER_LIBRARIES ${HEADER_NAME})
	endif ()
endforeach()

set(CONTAINER_LIBRARIES ${CONTAINER_LIBRARIES} PARENT_SCOPE)

# Install the built libraries
install(TARGETS ${CONTAINER_LIBRARIES}
    LIBRARY DESTINATION lib       # For shared libraries (.so/.dylib)
    ARCHIVE DESTINATION lib       # For static libraries (.a)
    RUNTIME DESTINATION bin)      # For executables (if any)

# Install header files (excluding the LinkDef.h files)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION include/Container
        FILES_MATCHING PATTERN "*.h"
        PATTERN "LinkDef.h" EXCLUDE)
