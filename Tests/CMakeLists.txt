include(CTest)

add_executable(Test main.cxx)
target_link_libraries(Test Launcher Fitter)

file(   COPY ${CMAKE_SOURCE_DIR}/Tests
        DESTINATION ${CMAKE_BINARY_DIR}/Tests)
file(   COPY ${CMAKE_SOURCE_DIR}/User/EXP/Template/Conf
        DESTINATION ${CMAKE_BINARY_DIR}/Tests/)
add_test(   NAME Agata-Prisma
            COMMAND ./Test  Agata-Prisma/COMM001/run_262)

#Google tests
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/bb9216085fbbf193408653ced9e73c61e7766e80.zip
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)
file(GLOB test_sources_list LIST_DIRECTORIES true src/*)
foreach(dir ${test_sources_list})
    if(IS_DIRECTORY ${dir})
        #get_filename_component(barename ${CMAKE_CURRENT_SOURCE_DIR} PATH)
        file(RELATIVE_PATH relpath ${CMAKE_SOURCE_DIR} ${dir})
        list(APPEND testdirlist ${relpath})
    else()
        continue()
    endif()
endforeach()

foreach(dir ${testdirlist})
    message(STATUS "Adding testing directory ${CMAKE_SOURCE_DIR}/${dir}")
    add_subdirectory(${CMAKE_SOURCE_DIR}/${dir})
endforeach()
