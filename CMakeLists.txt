cmake_minimum_required(VERSION 3.7.2 FATAL_ERROR)
project(RunSelector LANGUAGES CXX)

if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    endif()
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

# Optionally set a default install prefix (can be overridden at configure time)
if(NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation Prefix" FORCE)
endif()


#set -DEXP_NAME=YOUR_EXP_NAME to compile the user selector
set(EXP_NAME "Template" CACHE STRING "Set experiment name folder under User/EXP")
set(ENABLE_TESTING "OFF" CACHE BOOL "Enable testing")
set(USE_BOOST "OFF" CACHE BOOL "Enables boost libraries")
set(BUILD_SCRIPTS "OFF" CACHE BOOL "Builds scripts in the Scripts folder, useful for some analysis features/steps")
set(BUILD_BANANA_EDITOR "ON" CACHE BOOL "Build the banana editor")
set(USE_MPI "OFF" CACHE BOOL "Enable MPI for distributed computing (requires the installation of boost-mpi)")
set(USE_LINT "OFF" CACHE BOOL "Enables cxx lint")
set(BUILD_DOCUMENTATION "OFF" CACHE BOOL "Build doxygen documentation")
set(USE_CLANG_FORMAT "OFF" CACHE BOOL "Enables clang-format")
set(USE_CLANG_TIDY "OFF" CACHE BOOL "Enables clang-tidy")
set(COPY_CONF "OFF" CACHE BOOL "Copy the right conf in the build folder")
set(MAC_OSX "OFF" CACHE BOOL "Enables clang-tidy")
set(RENAME_CONTAINER_LIBRARIES "OFF" CACHE BOOL "Renames container libraries not to conflict with prisma library")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MAC_OSX TRUE)
endif()

#The spider calibration allows you to optimize the FWHM of the gamma peaks based on the position of spider
set(GRID_OPTIMIZER "OFF" CACHE BOOL "Choose if you want to produce grid optimizer executable - this is not the standard selector!")
if(GRID_OPTIMIZER)
    add_definitions(-DGRID_OPTIMIZER)
endif()

string(ASCII 27 Esc)
set(BOLDGREEN   "${Esc}[1;32m")
set(BOLDRED     "${Esc}[1;31m")
set(BOLDMAGENTA "${Esc}[1;35m")
set(COLORRESET  "${Esc}[m")

#Compiler flags
set(CMAKE_POSITION_INDEPENDENT_MODE ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_SHARED_LINKER_FLAGS "-W")
#set(CMAKE_CXX_FLAGS_DEBUG "-g -Wpedantic -Wconversion")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O3 -fmax-errors=5")
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (USE_LINT)
    set(CMAKE_CXX_CPPLINT "cpplint;--linelength=100")
endif ()

if(USE_CLANG_FORMAT)
    find_program(CLANG_FORMAT "clang-format")
    if(NOT CLANG_FORMAT)
        message(FATAL_ERROR "Could not find executable: clang-format")
    endif()
    # Get the output of the clang-format --version command
    execute_process(COMMAND ${CLANG_FORMAT} --version OUTPUT_VARIABLE CLANG_FORMAT_OUTPUT)

    # Extract the version number from the output
    string(REGEX MATCH "[0-9]+(\\.[0-9]+)*" CLANG_FORMAT_VERSION "${CLANG_FORMAT_OUTPUT}")

    # Compare the version with the minimum required version
    if(NOT "${CLANG_FORMAT_VERSION}" VERSION_GREATER "13.0")
        message(FATAL_ERROR "clang-format version: ${CLANG_FORMAT_VERSION} found, but version 13.0 or higher is required.")
    else()
        message("   clang-format is enabled, found version is ${CLANG_FORMAT_VERSION}")
        #file(GLOB_RECURSE ALL_SOURCE_FILES src/*.cxx src/*.h)
        file(GLOB_RECURSE ALL_SOURCE_FILES *.cxx *.h)
        list(FILTER ALL_SOURCE_FILES EXCLUDE REGEX "G__*")
        add_custom_target(
                clangformat
                COMMAND ${CLANG_FORMAT} --verbose --style=file -i ${ALL_SOURCE_FILES}
                COMMENT "Running clang-format"
        )
    endif()
endif()

if(USE_CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*")
endif()

if(USE_MPI)
    message("\n   ${BOLDMAGENTA}MPI has been enabled!\n${COLORESET}")
    set(USE_BOOST ON)
    add_definitions(-DUSE_MPI)
endif()

if(USE_BOOST)
    message("\n   ${BOLDMAGENTA}Boost has been enabled!\n${COLORESET}")
    set(Boost_USE_STATIC_LIBS OFF)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME OFF)
    #set(BOOST_ROOT "/op/opt/data/LNL/EXP_007/test/boost_install")
    if(USE_MPI)
        find_package(Boost REQUIRED mpi serialization)
    else ()
        find_package(Boost)
    endif()
    include_directories(${Boost_INCLUDE_DIRS})
    message("Boost libraries in ${Boost_INCLUDE_DIRS}")
endif()

if(MAC_OSX)
    add_definitions(-DMAC_OSX)
endif ()

#documentation, to generate: make doxygen
find_package( Doxygen )
if ( DOXYGEN_FOUND AND BUILD_DOCUMENTATION)
    message("Doxygen build started")
    set( DOXYGEN_OUTPUT_DIRECTORY doxygen )
    set( DOXYGEN_COLLABORATION_GRAPH YES )
    set( DOXYGEN_EXTRACT_ALL YES )
    set( DOXYGEN_CLASS_DIAGRAMS YES )
    set( DOXYGEN_HIDE_UNDOC_RELATIONS NO )
    set( DOXYGEN_HAVE_DOT YES )
    set( DOXYGEN_CLASS_GRAPH YES )
    set( DOXYGEN_CALL_GRAPH YES )
    set( DOXYGEN_CALLER_GRAPH YES )
    set( DOXYGEN_COLLABORATION_GRAPH YES )
    set( DOXYGEN_BUILTIN_STL_SUPPORT YES )
    set( DOXYGEN_EXTRACT_PRIVATE YES )
    set( DOXYGEN_EXTRACT_PACKAGE YES )
    set( DOXYGEN_EXTRACT_STATIC YES )
    set( DOXYGEN_EXTRACT_LOCALMETHODS YES )
    set( DOXYGEN_UML_LOOK YES )
    set( DOXYGEN_UML_LIMIT_NUM_FIELDS 50 )
    set( DOXYGEN_TEMPLATE_RELATIONS YES )
    set( DOXYGEN_DOT_GRAPH_MAX_NODES 100 )
    set( DOXYGEN_MAX_DOT_GRAPH_DEPTH 0 )
    set( DOXYGEN_DOT_TRANSPARENT YES )
    doxygen_add_docs( doxygen ${CMAKE_SOURCE_DIR} )
else()
    #message( "Doxygen need to be installed to generate the doxygen documentation" )
endif()

#ROOT includes
list(APPEND CMAKE_PREFIX_PATH "$ENV{ROOTSYS}/cmake")
set(ROOT_DIR "$ENV{ROOTSYS}/cmake")
message("ROOT_DIR is ${ROOT_DIR}")
find_package(ROOT REQUIRED COMPONENTS   Core Gui Imt RIO Net Hist MathCore Graf Graf3d Gpad
                                        Tree TreePlayer Rint Postscript Matrix Physics Thread
                                        Matrix MultiProc 
                                        RooFitCore RooFit
					ROOTDataFrame ROOTVecOps Minuit2)
include(${ROOT_USE_FILE})
include_directories(${ROOT_INCLUDE_DIRS})
add_definitions(${ROOT_CXX_FLAGS})


# Get the current working branch
execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

message("   Git commit hash is : ${GIT_COMMIT_HASH}")
message("   Git branch is : ${GIT_BRANCH}")
add_definitions("-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
add_definitions("-DGIT_BRANCH=\"${GIT_BRANCH}\"")

#Check if cxx version of root is compatible with the selector

string(FIND ${ROOT_CXX_FLAGS} "-std=c++11" ROOT_CXX_11)
if(${ROOT_CXX_11} GREATER_EQUAL 1)
    set(CMAKE_CXX_STANDARD 11)
endif()

string(FIND ${ROOT_CXX_FLAGS} "-std=c++14" ROOT_CXX_14)
if(${ROOT_CXX_14} GREATER_EQUAL 1)
    if(GRID_OPTIMIZER)
        message(FATAL_ERROR "\n   ${BOLDRED}The grid optimizer needs ROOT to be built with at least C++ 17\n${COLORRESET}")
    endif ()
	set(CMAKE_CXX_STANDARD 14)
endif()

string(FIND ${ROOT_CXX_FLAGS} "-std=c++17" ROOT_CXX_17)
if(${ROOT_CXX_17} GREATER_EQUAL 1)
	set(CMAKE_CXX_STANDARD 17)
endif()

string(FIND ${ROOT_CXX_FLAGS} "-std=c++2a" ROOT_CXX_20)
if(${ROOT_CXX_20} GREATER_EQUAL 1)
	set(CMAKE_CXX_STANDARD 20)
endif()

if(${CMAKE_CXX_STANDARD} EQUAL 11)
  message(FATAL_ERROR "\n   ${BOLDRED}C++ version of ROOT should be more recent, compile it with at least cxx 14\n${COLORRESET}")
else()
	message("\n   ${BOLDRED}C++ version of ROOT set to: ${CMAKE_CXX_STANDARD}\n${COLORRESET}")
endif()


if(NOT GRID_OPTIMIZER)
    set(EXP_FOLDER EXP)
else ()
    set(EXP_FOLDER grid_optimizer)
    set(EXP_NAME Conf_and_DummySelector)
endif()

#User selector
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/User/${EXP_FOLDER}/${EXP_NAME})
    file(GLOB user_files LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/User/${EXP_FOLDER}/Template/*)
    foreach(file ${user_files})
        file(COPY ${file} DESTINATION ${CMAKE_SOURCE_DIR}/User/${EXP_FOLDER}/${EXP_NAME})
    endforeach()
endif()

include_directories(${RunSelector_SOURCE_DIR}/User/${EXP_FOLDER}/${EXP_NAME})
set(USER_LIBRARIES)

#Core
include_directories("${CMAKE_SOURCE_DIR}/src/Core")
set(CORE_LIBRARIES)
#Lut
include_directories("${CMAKE_SOURCE_DIR}/src/Lut")
set(LUT_LIBRARIES)
#Conf
include_directories("${CMAKE_SOURCE_DIR}/src/Conf")
set(CONF_LIBRARIES)
#Container
include_directories("${CMAKE_SOURCE_DIR}/src/Container")
set(CONTAINER_LIBRARIES)
#Physics
include_directories("${CMAKE_SOURCE_DIR}/src/Physics")
set(PHYSICS_LIBRARIES)
#Tools
include_directories("${CMAKE_SOURCE_DIR}/src/Tools")
set(TOOLS_LIBRARIES)
#Selector
include_directories("${CMAKE_SOURCE_DIR}/src/Selector")
set(SELECTOR_LIBRARIES)
#Optimizer
include_directories("${CMAKE_SOURCE_DIR}/src/Optimizer")
set(OPTIMIZER_LIBRARIES)
#DataStructures
include_directories("${CMAKE_SOURCE_DIR}/src/DataStructures")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/src/DataStructures"
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")


#Conf
link_directories("${CMAKE_BINARY_DIR}/src/Conf")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/Conf")

#Container
link_directories("${CMAKE_BINARY_DIR}/src/Container")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/Container")

#Lut
link_directories("${CMAKE_BINARY_DIR}/src/Lut")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/Lut")

#Physics
link_directories("${CMAKE_BINARY_DIR}/src/Physics")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/Physics")

#Tools
link_directories("${CMAKE_BINARY_DIR}/src/Tools")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/Tools")

#Selector
link_directories("${CMAKE_BINARY_DIR}/src/Selector")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/Selector")

#Optimizer
link_directories("${CMAKE_BINARY_DIR}/src/Optimizer")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/Optimizer")

link_directories(${CMAKE_BINARY_DIR}/User/${EXP_FOLDER}/${EXP_NAME})
add_subdirectory(${CMAKE_SOURCE_DIR}/User/${EXP_FOLDER}/${EXP_NAME})

#Core
link_directories("${CMAKE_BINARY_DIR}/src/Core")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/Core")


#Tests
if(ENABLE_TESTING)
    message("\n   ${BOLDMAGENTA}Testing has been enabled!\n${COLORESET}")
    add_subdirectory(Tests)
    enable_testing()
endif()

#Copy conf
if(COPY_CONF)
    if(NOT EXISTS ${CMAKE_BINARY_DIR}/Conf)
        file(COPY ${CMAKE_SOURCE_DIR}/User/EXP/${EXP_NAME}/Conf DESTINATION ${CMAKE_BINARY_DIR})
    endif()
endif()


if(NOT GRID_OPTIMIZER)
    #default compilation
    message("\n   ${BOLDGREEN}I will compile standard AgataSelector!\n${COLORRESET}")
    message("\n   ${BOLDRED}Experiment set to: ${EXP_NAME}\n${COLORRESET}")
    if(USE_MPI)
        add_executable(__MPI_RunSelector__ src/main.cxx)
        target_link_libraries(__MPI_RunSelector__ App Optimizer Settings ${Boost_LIBRARIES})
        add_executable(RunSelector src/mainMpi.cxx)

        # Install executable targets
        install(TARGETS RunSelector __MPI_RunSelector__ RUNTIME DESTINATION bin)


        # Install the executable into bin:
    else()
        add_executable(RunSelector src/main.cxx)
        target_link_libraries(RunSelector App Optimizer Settings ${Boost_LIBRARIES})
        install(TARGETS RunSelector RUNTIME DESTINATION bin)
    endif()
else()
    #compilation with SPIDER angular calibration
    message("\n   ${BOLDGREEN}I will compile Grid Optimizer!\n${COLORRESET}")
    add_subdirectory(${CMAKE_SOURCE_DIR}/User/grid_optimizer/)
    link_directories(${CMAKE_BINARY_DIR}/User/grid_optimizer/)
endif()
    # Optionally, install any scripts as well
    install(FILES ${CMAKE_SOURCE_DIR}/Scripts/selector.sh
        DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )

add_subdirectory("${CMAKE_SOURCE_DIR}/Scripts")


# Banana editor
if(BUILD_BANANA_EDITOR)
    set(BANANA_EDITOR_SOURCE_DIR "${CMAKE_SOURCE_DIR}/banana-editor")

    if(NOT EXISTS "${BANANA_EDITOR_SOURCE_DIR}/CMakeLists.txt")
        find_package(Git REQUIRED)
        message(STATUS "Banana editor sources not found; cloning the submodule")

        execute_process(
            COMMAND "${GIT_EXECUTABLE}" submodule update --init --recursive -- banana-editor
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            RESULT_VARIABLE BANANA_EDITOR_SUBMODULE_RESULT
        )

        # Older checkouts may contain the .gitmodules entry without a gitlink.
        # In that case, obtain the repository URL from .gitmodules and clone it.
        if(NOT EXISTS "${BANANA_EDITOR_SOURCE_DIR}/CMakeLists.txt")
            execute_process(
                COMMAND "${GIT_EXECUTABLE}" config --file "${CMAKE_SOURCE_DIR}/.gitmodules"
                        --get submodule.banana-editor.url
                OUTPUT_VARIABLE BANANA_EDITOR_GIT_URL
                OUTPUT_STRIP_TRAILING_WHITESPACE
                RESULT_VARIABLE BANANA_EDITOR_URL_RESULT
            )

            if(NOT BANANA_EDITOR_URL_RESULT EQUAL 0 OR BANANA_EDITOR_GIT_URL STREQUAL "")
                message(FATAL_ERROR "Could not determine the banana editor repository URL from .gitmodules")
            endif()

            execute_process(
                COMMAND "${GIT_EXECUTABLE}" clone --recurse-submodules
                        "${BANANA_EDITOR_GIT_URL}" "${BANANA_EDITOR_SOURCE_DIR}"
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                RESULT_VARIABLE BANANA_EDITOR_CLONE_RESULT
            )

            if(NOT BANANA_EDITOR_CLONE_RESULT EQUAL 0)
                message(FATAL_ERROR "Failed to clone the banana editor repository")
            endif()
        elseif(NOT BANANA_EDITOR_SUBMODULE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to initialize the banana editor submodule")
        endif()
    endif()

    set(BANANA_EDITOR_BUILD_OSCAR_EDITOR ON CACHE BOOL "" FORCE)
    set(BANANA_EDITOR_BUILD_KRATTA_EDITOR OFF CACHE BOOL "" FORCE)
    add_subdirectory("${BANANA_EDITOR_SOURCE_DIR}")
endif()
