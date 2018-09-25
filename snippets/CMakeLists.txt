cmake_minimum_required(VERSION 2.8.12)

set(PROJECT_SNIPPETS_NAME ${PROJECT_NAME}_snippets)

###########################################################################################

file(GLOB_RECURSE SNIPPETS_SOURCE_FILES ${PROJECT_SOURCE_DIR}/snippets/src/*(.cpp,.cc,.cxx,.hpp,.hxx,.c,.h))

add_executable(${PROJECT_SNIPPETS_NAME} ${SNIPPETS_SOURCE_FILES})

#add libtins
target_link_libraries(${PROJECT_SNIPPETS_NAME} VirtualStack)
target_include_directories(${PROJECT_SNIPPETS_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)

#add rawSocket capabilities
#SET(CAP_ROOT_PW think)
#add_custom_command(
#        TARGET ${PROJECT_SNIPPETS_NAME}
#        POST_BUILD COMMAND ./setCapabilites.sh ${CAP_ROOT_PW} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_SNIPPETS_NAME}
#        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#        COMMENT "Set permissions to access raw_socket without root, if this takes long or fails you need to set a password in snippets/CmakeLists.txt")