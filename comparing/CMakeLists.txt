cmake_minimum_required(VERSION 2.8.12)

set(PROJECT_COMPARING_NAME ${PROJECT_NAME}_comparing)

###########################################################################################

file(GLOB_RECURSE COMPARING_SOURCE_FILES ${PROJECT_SOURCE_DIR}/comparing/src/*(.cpp,.cc,.cxx,.hpp,.hxx,.c,.h))

add_executable(${PROJECT_COMPARING_NAME} ${COMPARING_SOURCE_FILES})

#add libtins
target_link_libraries(${PROJECT_COMPARING_NAME} VirtualStack)
target_include_directories(${PROJECT_COMPARING_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)