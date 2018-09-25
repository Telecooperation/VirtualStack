cmake_minimum_required(VERSION 2.8.12)

find_package(Threads)

set(PROJECT_RUNNER_NAME ${PROJECT_NAME}_runner)

###########################################################################################

file(GLOB_RECURSE RUNNER_SOURCE_FILES ${PROJECT_SOURCE_DIR}/runner/src/*(.cpp, .h))

add_executable(${PROJECT_RUNNER_NAME} ${RUNNER_SOURCE_FILES})

target_link_libraries(${PROJECT_RUNNER_NAME} VirtualStack)
target_include_directories(${PROJECT_RUNNER_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)