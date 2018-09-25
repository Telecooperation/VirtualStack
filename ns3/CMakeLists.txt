cmake_minimum_required(VERSION 2.8.12)

find_package(Threads)

set(PROJECT_NS3_NAME ${PROJECT_NAME}_ns3)

set(LIB_DIR "${PROJECT_SOURCE_DIR}/ns3/lib")
set(LIB_NS3_SOURCE "${LIB_DIR}/libns3")
set(LIB_NS3_INSTALL "${LIB_NS3_SOURCE}/install")
set(LIB_NS3_DCE_SOURCE "${LIB_DIR}/libns3-dce")
set(LIB_NS3_DCE_INSTALL "${LIB_NS3_DCE_SOURCE}/install")

set(NS3_PATH "${LIB_NS3_INSTALL}")
set(NS3_DCE_PATH "${LIB_NS3_DCE_INSTALL}")
set(NS3_VERSION "3-dev")
set(NS3_PROFILE "optimized")

# Is disabled for performance purpose. After change the Cmake cache has to be reset
#SET(NS3_LOG_ENABLE TRUE)
#SET(NS3_ASSERT_ENABLE TRUE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/ns3/cmake")

file(MAKE_DIRECTORY ${BINARY_DIR}/sim)

# REMBEMBER: put "optimized" into NS3_PROFILE if --build-profile=optimized
#--build-profile=optimized
#--enable-sudo for tap-device
#--disable-examples
add_custom_target(build_ns3
        COMMAND python2 ./waf configure --disable-tests --disable-examples --build-profile=optimized --prefix=${LIB_NS3_INSTALL} build install
        COMMAND cp ${LIB_NS3_INSTALL}/bin/ns${NS3_VERSION}-tap-creator-${NS3_PROFILE} ${BINARY_DIR}
        COMMAND sudo setcap cap_net_raw,cap_net_admin=ep  ${BINARY_DIR}/ns${NS3_VERSION}-tap-creator-${NS3_PROFILE}
        WORKING_DIRECTORY ${LIB_NS3_SOURCE})

set(PATCH_NS3_ARGS_1 "'s@    build_dce_tests(module, bld)@    \#build_dce_tests(module, bld)@g'")
set(PATCH_NS3_ARGS_2 "'s@    build_dce_examples(module, bld)@    \#build_dce_examples(module, bld)@g'")
set(PATCH_NS3_DCE_NULL_BIND "'s@= this->m_kernsock->Bind (NULL, 0)\;@= 0\; //this->m_kernsock->Bind (NULL, 0)\;@g'")

add_custom_target(patch_ns3-dce-build
        COMMAND sed -i -e ${PATCH_NS3_ARGS_1} wscript
        COMMAND sed -i -e ${PATCH_NS3_ARGS_2} wscript
        WORKING_DIRECTORY ${LIB_NS3_DCE_SOURCE})

add_custom_target(patch_ns3-dce_null_bind
        COMMAND sed -i -e ${PATCH_NS3_DCE_NULL_BIND} model/linux/linux-socket-impl.cc
        WORKING_DIRECTORY ${LIB_NS3_DCE_SOURCE})

# --enable-opt
# --enable-kernel-stack
#CXX="g++-5" CC="gcc-5" python2 ./waf
#
add_custom_target(build_ns3-dce
        COMMAND python2 ./waf configure --disable-tests --enable-opt --enable-kernel-stack=${LIB_DIR}/net-next-nuse/arch --with-ns3=${LIB_NS3_INSTALL} --prefix=${LIB_NS3_DCE_INSTALL} build install
        COMMAND cp ${LIB_NS3_DCE_INSTALL}/lib/*.so ${BINARY_DIR}
        COMMAND cp ${LIB_DIR}/liblinux.so ${BINARY_DIR}
        COMMAND cp ${LIB_DIR}/ip ${BINARY_DIR}
        DEPENDS patch_ns3-dce_null_bind patch_ns3-dce-build build_ns3
        WORKING_DIRECTORY ${LIB_NS3_DCE_SOURCE})


FIND_PACKAGE(NS3 COMPONENTS
        applications
        mpi bridge flow-monitor traffic-control
        wifi lte wimax csma uan wave tap-bridge virtual-net-device
        mobility
        point-to-point
        internet network
        core netanim internet-apps
        REQUIRED)

FIND_PACKAGE(NS3_DCE)

link_directories(${NS3_DCE_LIBRARY_DIRS})
add_definitions(${NS3_DCE_DEFINITIONS})

link_directories(${NS3_LIBRARY_DIRS})
add_definitions(${NS3_DEFINITIONS})

if(NOT NS3_FOUND)
    message(WARNING " NS3 not found. You have to call build_ns3 and rerun cmake")
endif()

if(NOT NS3_DCE_FOUND)
    message(WARNING " NS3-DCE not found. You have to call build_ns3-dce and rerun cmake")
    message(WARNING "To compile NS3 you need to run \"make build_ns3-dce\" with GCC Version <= 5. In ns3/CMakeList.txt you can prepend CXX=\"g++-5\" CC=\"gcc-5\" before python2 ./waf on the build_ns3-dce COMMAND")
endif()

if(NS3_FOUND)
    file(GLOB_RECURSE NS3_SOURCE_FILES ${PROJECT_SOURCE_DIR}/ns3/src/*(.cpp, .h))

    add_executable(${PROJECT_NS3_NAME} ${NS3_SOURCE_FILES})

    target_compile_options(${PROJECT_NS3_NAME} PRIVATE -Wno-unknown-warning-option -Wno-zero-as-null-pointer-constant -Wno-float-equal)
    target_include_directories(${PROJECT_NS3_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src) #VirtualStack
    target_include_directories(${PROJECT_NS3_NAME} SYSTEM PRIVATE ${NS3_INCLUDE_DIR})

    target_link_libraries(${PROJECT_NS3_NAME} VirtualStack)
    target_link_libraries(${PROJECT_NS3_NAME} ${CMAKE_THREAD_LIBS_INIT} ${NS3_LIBRARIES})

    if(NS3_DCE_FOUND)
        target_include_directories(${PROJECT_NS3_NAME} SYSTEM PRIVATE ${NS3_DCE_INCLUDE_DIR})
        target_link_libraries(${PROJECT_NS3_NAME} ${NS3_DCE_LIBRARIES})
    endif()
endif()