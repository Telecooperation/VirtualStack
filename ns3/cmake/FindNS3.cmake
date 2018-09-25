#INCLUDE(SelectLibraryConfigurations)
#INCLUDE(FindPackageHandleStandardArgs)

#set(NS3_VERSION "3.26")
#set(NS3_PROFILE "debug")

IF(NOT NS3_VERSION)
	MESSAGE(FATAL_ERROR "NS3_VERSION not set. Example: set(NS3_VERSION \"3.26\" or set(NS3_VERSION \"3-dev\")")
ENDIF()

IF(NOT NS3_PROFILE)
	MESSAGE(FATAL_ERROR "NS3_PROFILE not set. Example: set(NS3_PROFILE \"debug\")")
ENDIF()

IF(NS3_ASSERT_ENABLE)
	set(NS3_DEFINITIONS ${NS3_DEFINITIONS}
			-DNS3_ASSERT_ENABLE)
ENDIF()

IF(NS3_LOG_ENABLE)
	set(NS3_DEFINITIONS ${NS3_DEFINITIONS}
			-DNS3_LOG_ENABLE)
ENDIF()

# List of the valid ns3 components.
SET(NS3_VALID_COMPONENTS
		antenna
		aodv
		applications
		bridge
		buildings
		config-store
		core
		csma
		csma-layout
		dsdv
		dsr
		energy
		fd-net-device
		flow-monitor
		internet-apps
		internet
		lr-wpan
		lte
		mesh
		mobility
		mpi
		netanim
		network
		nix-vector-routing
		olsr
		point-to-point
		point-to-point-layout
		propagation
		sixlowpan
		spectrum
		stats
		tap-bridge
		test
		topology-read
		traffic-control
		uan
		virtual-net-device
		wave
		wifi
		wimax
)

# Find the ns3 core library.
FIND_LIBRARY(NS3_LIBRARIES
	NAME ns${NS3_VERSION}-core-${NS3_PROFILE}
	HINTS
	${NS3_PATH}
	$ENV{LD_LIBRARY_PATH}
	$ENV{NS3_PATH}
	/usr/lib
    /usr/local/lib
	PATH_SUFFIXES lib64 lib ns3/lib
	PATHS
	/opt
	/opt/local
	/opt/csw
	/sw
	/usr/local
)

# Find the include dir for ns3.
FIND_PATH(NS3_INCLUDE_DIR
	NAME ns3/core-module.h
	HINTS
	${NS3_PATH}
	$ENV{NS3_PATH}
    /usr/lib
    /usr/local/lib
	PATH_SUFFIXES include ns3/include ns${NS3_VERSION} include/ns${NS3_VERSION}/
	PATHS
	/opt
	/opt/local
	/opt/csw
	/sw
	/usr/local
)

IF(NS3_INCLUDE_DIR)
	MESSAGE(STATUS "Looking for core-module.h - found")
ELSE()
	MESSAGE(STATUS "Looking for core-module.h - not found")
ENDIF()

IF(NS3_LIBRARIES)
	MESSAGE(STATUS "Looking for lib ns3 - found")
ELSE()
	MESSAGE(STATUS "Looking for lib ns3 - not found")
ENDIF()

# Validate the list of find components.
#SET(NS3_CHOSEN_COMPONENTS "core")
IF(NS3_FIND_COMPONENTS)
	FOREACH(component ${NS3_FIND_COMPONENTS})
		LIST(FIND NS3_VALID_COMPONENTS ${component} component_location)
		IF(${component_location} EQUAL -1)
			MESSAGE(FATAL_ERROR "\"${component}\" is not a valid NS3 component.")
		ELSE()
			LIST(FIND NS3_CHOSEN_COMPONENTS ${component} component_location)
			IF(${component_location} EQUAL -1)
				LIST(APPEND NS3_CHOSEN_COMPONENTS ${component})
			ENDIF()
		ENDIF()
	ENDFOREACH()
ENDIF()

# Library + Include Directories
IF(NS3_LIBRARIES AND NS3_INCLUDE_DIR)
 	GET_FILENAME_COMPONENT(NS3_LIBRARY_DIR ${NS3_LIBRARIES} PATH)
	MESSAGE(STATUS "NS3 Library directory is ${NS3_LIBRARY_DIR}")
	STRING(REGEX MATCH "${NS3_LIBRARY_DIR}" in_path "$ENV{LD_LIBRARY_PATH}")

	IF(NOT in_path)
		#MESSAGE(STATUS "Warning: To use NS-3 don't forget to set LD_LIBRARY_PATH with:	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${NS3_LIBRARY_DIR}")
	ELSE()
		STRING(REGEX MATCH "-L${NS3_LIBRARY_DIR} " have_Lflag "${CMAKE_C_FLAGS}")
		IF(NOT have_Lflag)
			SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}-L${NS3_LIBRARY_DIR} ")
		ENDIF()

		STRING(REGEX MATCH "-I${NS3_INCLUDE_DIR} " have_Iflag "${CMAKE_C_FLAGS}")
		IF(NOT have_Iflag`)
			SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}-I${NS3_INCLUDE_DIR} ")
		ENDIF()

		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-I${NS3_INCLUDE_DIR} -L${NS3_LIBRARY_DIR} ")
	ENDIF()
	SET(NS3_FOUND 1)
ENDIF()

# Try to find components
IF(NS3_FOUND)
 	FOREACH(_component ${NS3_CHOSEN_COMPONENTS})
		FIND_LIBRARY(NS3_${_component}_LIBRARY
			NAME ns${NS3_VERSION}-${_component}-${NS3_PROFILE}
			HINTS
			${NS3_PATH}
			$ENV{LD_LIBRARY_PATH}
			$ENV{NS3_PATH}
			/usr/lib
            /usr/local/lib
			PATH_SUFFIXES lib64 lib ns3/lib
			PATHS
			/opt
			/opt/local
			/opt/csw
			/sw
			/usr/local
			${NS3_LIBRARY_DIR}
		)
		MARK_AS_ADVANCED(NS3_${_component}_LIBRARY)
		LIST(APPEND NS3_LIBRARIES ${NS3_${_component}_LIBRARY})
	ENDFOREACH()
ENDIF()

#FIND_PACKAGE_HANDLE_STANDARD_ARGS(NS3 DEFAULT_MSG NS3_LIBRARIES NS3_INCLUDE_DIR)
#MARK_AS_ADVANCED(NS3_LIBRARIES NS3_INCLUDE_DIR)