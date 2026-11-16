add_library(EdECalculator SHARED EdECalculator.cxx)

target_include_directories(EdECalculator
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(EdECalculator
    PUBLIC
        ${ROOT_LIBRARIES}
        ${PHYSICS_LIBRARIES}
)

add_library(EdECalculatorOscar SHARED EdECalculatorOscar.cxx)

target_include_directories(EdECalculatorOscar
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(EdECalculatorOscar
    PUBLIC
        EdECalculator
        ${ROOT_LIBRARIES}
        ${CONF_LIBRARIES}
        ${LUT_LIBRARIES}
)

set(EDE_CALCULATOR_LIBRARIES EdECalculator EdECalculatorOscar PARENT_SCOPE)
