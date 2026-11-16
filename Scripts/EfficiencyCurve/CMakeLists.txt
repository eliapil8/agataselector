file(GLOB SRC
     CONFIGURE_DEPENDS
     "*.cxx")

add_executable(EfficiencyCurve ${SRC} ${ROOT_LIBRARIES})
target_link_libraries(EfficiencyCurve ${ROOT_LIBRARIES} ${OPTIMIZER_LIBRARIES})

file(COPY "files" DESTINATION "${CMAKE_BINARY_DIR}/Scripts/EfficiencyCurve/")