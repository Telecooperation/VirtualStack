cmake_minimum_required(VERSION 2.8.12)
option(USE_CLANG "build application with clang" ON) # OFF is the default

#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(USE_CLANG)
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
    #DONT USE, only usefull for MACOSX set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wall -Wextra -Wno-padded -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-exit-time-destructors -Wno-global-constructors -Wno-newline-eof -Wno-missing-braces -Wno-missing-prototypes -Wno-packed -Wno-unknown-warning-option -Wno-zero-length-array")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-unused-parameter -Wno-padded")
else()
    set(CMAKE_C_COMPILER "gcc")
    set(CMAKE_CXX_COMPILER "g++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fPIC -pie -rdynamic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-unused-parameter")
endif()

set(CUSTOM_RELEASE_FLAGS
        -O3
        -funroll-loops
        -DNDEBUG)

set(CUSTOM_RELWITHDEBUG_FLAGS
        -O3
        -funroll-loops
        -ftree-vectorize)

project(VirtualStack)

set(BINARY_DIR "${PROJECT_SOURCE_DIR}/bin")
file(MAKE_DIRECTORY ${BINARY_DIR})
set(CMAKE_CURRENT_BINARY_DIR ${BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARY_DIR})

OPTION(AVX "Build project with AVX support." ON)

if(AVX)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mavx")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -mavx")
endif(AVX)

# --system-header-prefix="
#  -fsanitize=address
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-unused-parameter -Wno-switch-enum -fPIC")

find_package(Threads)

###########################################################################################

## ProjectSettings
set(CODE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")

file(GLOB_RECURSE SOURCE_FILES ${CODE_SOURCE_DIR}/*(.cpp,.cc,.cxx,.hpp,.hxx,.c,.h))

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:${CUSTOM_RELWITHDEBUG_FLAGS}>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${CUSTOM_RELEASE_FLAGS}>")

###########################################################################################

IF(EXISTS "${PROJECT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
ENDIF()

IF(EXISTS "${PROJECT_SOURCE_DIR}/runner/CMakeLists.txt")
    add_subdirectory(runner)
ENDIF()

IF(EXISTS "${PROJECT_SOURCE_DIR}/snippets/CMakeLists.txt")
    add_subdirectory(snippets)
ENDIF()

IF(EXISTS "${PROJECT_SOURCE_DIR}/comparing/CMakeLists.txt")
    add_subdirectory(comparing)
ENDIF()

IF(EXISTS "${PROJECT_SOURCE_DIR}/ns3/CMakeLists.txt")
    add_subdirectory(ns3)
ENDIF()
