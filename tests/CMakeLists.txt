cmake_minimum_required(VERSION 2.8.12)

IF(EXISTS "${PROJECT_SOURCE_DIR}/tests/lib/googletest/CMakeLists.txt")
    add_subdirectory(lib/googletest
            EXCLUDE_FROM_ALL)

    set(PROJECT_TEST_NAME ${PROJECT_NAME}_test)

    ###########################################################################################

    file(GLOB_RECURSE TEST_SOURCE_FILES ${PROJECT_SOURCE_DIR}/tests/src/*(.cpp, .h))

    add_executable(${PROJECT_TEST_NAME} ${TEST_SOURCE_FILES})

    target_compile_options(gtest PRIVATE -w)
    target_compile_options(gtest_main PRIVATE -w)
    target_compile_options(gmock PRIVATE -w)
    target_compile_options(gmock_main PRIVATE -w)

    target_compile_options(${PROJECT_TEST_NAME} PRIVATE
            -Wno-zero-as-null-pointer-constant
            -Wno-unused-function)

    target_include_directories(${PROJECT_TEST_NAME} PRIVATE "../src")
    target_include_directories(${PROJECT_TEST_NAME} SYSTEM PRIVATE "lib/googletest/googletest/include")
    target_include_directories(${PROJECT_TEST_NAME} SYSTEM PRIVATE "lib/googletest/googlemock/include")

    target_link_libraries(${PROJECT_TEST_NAME} VirtualStack gmock gmock_main)
ENDIF()