INCLUDE(SelectLibraryConfigurations)
INCLUDE(FindPackageHandleStandardArgs)

# List of the valid ns3 components.
SET(NS3_DCE_VALID_COMPONENTS
		c-ns3
		dl-ns3
		m-ns3
		mylib
		ns3-dce
		ns3-dce-test-netlink
		ns3-netlink
		pthread-ns3
		rt-ns3
)

# Find the ns3 core library.
FIND_LIBRARY(NS3_DCE_LIBRARIES
	NAME ns3-dce
	HINTS
	${NS3_DCE_PATH}
	$ENV{LD_LIBRARY_PATH}
	$ENV{NS3_DCE_PATH}
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
FIND_PATH(NS3_DCE_INCLUDE_DIR
	NAME ns3/dce-module.h
	HINTS
	${NS3_DCE_PATH}
	$ENV{NS3_DCE_PATH}
    /usr/lib
    /usr/local/lib
	PATH_SUFFIXES include ns3/include ns3 include/ns3/
	PATHS
	/opt
	/opt/local
	/opt/csw
	/sw
	/usr/local
)

IF(NS3_DCE_INCLUDE_DIR)
	MESSAGE(STATUS "Looking for dce-module.h - found")
ELSE()
	MESSAGE(STATUS "Looking for dce-module.h - not found")
ENDIF()

IF(NS3_DCE_LIBRARIES)
	MESSAGE(STATUS "Looking for lib ns3-dce - found")
ELSE()
	MESSAGE(STATUS "Looking for lib ns3-dce - not found")
ENDIF()

# Validate the list of find components.
SET(NS3_DCE_CHOSEN_COMPONENTS "ns3-dce")
IF(NS3_DCE_FIND_COMPONENTS)
	FOREACH(component ${NS3_DCE_FIND_COMPONENTS})
		LIST(FIND NS3_DCE_VALID_COMPONENTS ${component} component_location)
		IF(${component_location} EQUAL -1)
			MESSAGE(FATAL_ERROR "\"${component}\" is not a valid NS3-dce component.")
		ELSE()
			LIST(FIND NS3_DCE_CHOSEN_COMPONENTS ${component} component_location)
			IF(${component_location} EQUAL -1)
				LIST(APPEND NS3_DCE_CHOSEN_COMPONENTS ${component})
			ENDIF()
		ENDIF()
	ENDFOREACH()
ENDIF()

# Library + Include Directories
IF(NS3_DCE_LIBRARIES AND NS3_DCE_INCLUDE_DIR)
 	GET_FILENAME_COMPONENT(NS3_DCE_LIBRARY_DIR ${NS3_DCE_LIBRARIES} PATH)
	MESSAGE(STATUS "NS3-dce Library directory is ${NS3_DCE_LIBRARY_DIR}")
	STRING(REGEX MATCH "${NS3_DCE_LIBRARY_DIR}" in_path "$ENV{LD_LIBRARY_PATH}")

	IF(NOT in_path)
		#MESSAGE(STATUS "Warning: To use NS-3 don't forget to set LD_LIBRARY_PATH with:	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${NS3_DCE_LIBRARY_DIR}")
	ELSE()
		STRING(REGEX MATCH "-L${NS3_DCE_LIBRARY_DIR} " have_Lflag "${CMAKE_C_FLAGS}")
		IF(NOT have_Lflag)
			SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}-L${NS3_DCE_LIBRARY_DIR} ")
		ENDIF()

		STRING(REGEX MATCH "-I${NS3_DCE_INCLUDE_DIR} " have_Iflag "${CMAKE_C_FLAGS}")
		IF(NOT have_Iflag`)
			SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}-I${NS3_DCE_INCLUDE_DIR} ")
		ENDIF()

		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-I${NS3_DCE_INCLUDE_DIR} -L${NS3_DCE_LIBRARY_DIR} ")
	ENDIF()
	SET(NS3_DCE_FOUND 1)
ENDIF()

# Try to find components
IF(NS3_DCE_FOUND)
 	FOREACH(_component ${NS3_DCE_CHOSEN_COMPONENTS})
		FIND_LIBRARY(NS3_DCE_${_component}_LIBRARY
			NAME ${_component}
			HINTS
			${NS3_DCE_PATH}
			$ENV{LD_LIBRARY_PATH}
			$ENV{NS3_DCE_PATH}
			/usr/lib
            /usr/local/lib
			PATH_SUFFIXES lib64 lib ns3/lib
			PATHS
			/opt
			/opt/local
			/opt/csw
			/sw
			/usr/local
			${NS3_DCE_LIBRARY_DIR}
		)
		MARK_AS_ADVANCED(NS3_DCE_${_component}_LIBRARY)
		LIST(APPEND NS3_DCE_LIBRARIES ${NS3_DCE_${_component}_LIBRARY})
	ENDFOREACH()
ENDIF()

#FIND_PACKAGE_HANDLE_STANDARD_ARGS(NS3 DEFAULT_MSG NS3_DCE_LIBRARIES NS3_DCE_INCLUDE_DIR)
#MARK_AS_ADVANCED(NS3_DCE_LIBRARIES NS3_DCE_INCLUDE_DIR)