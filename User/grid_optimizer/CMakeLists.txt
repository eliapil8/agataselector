include_directories(${RunSelector_SOURCE_DIR}/User/grid_optimizer)
link_directories(${CMAKE_BINARY_DIR}/User/grid_optimizer)

ROOT_GENERATE_DICTIONARY(G__TPositionOffset TPositionOffset.hpp LINKDEF TPositionOffsetLinkDef.h)
add_library(TPositionOffset SHARED G__TPositionOffset.cxx)
target_link_libraries(TPositionOffset ${ROOT_LIBRARIES} )

ROOT_GENERATE_DICTIONARY(G__TOptiData TOptiData.hpp LINKDEF TOptiDataLinkDef.h)
add_library(TOptiData SHARED G__TOptiData.cxx)
target_link_libraries(TOptiData ${ROOT_LIBRARIES} )

add_executable(RunGridOptimizer GridOptimizer.cpp OptiSpider.hpp ROOTFileReducer.hpp ROOTFileMerger.hpp OptiSegment.hpp OptiDetector.hpp OptiFitter.hpp)
set_target_properties(RunGridOptimizer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Add this line
target_include_directories(RunGridOptimizer PRIVATE ${ROOT_INCLUDE_DIRS})

target_link_libraries(RunGridOptimizer Container TPositionOffset TOptiData App ${ROOT_LIBRARIES})
