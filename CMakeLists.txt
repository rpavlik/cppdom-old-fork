# CMake-based build system for cppdom
# 2009-2011 Ryan Pavlik <rpavlik@iastate.edu>
# http://academic.cleardefinition.com/
# Iowa State University HCI Graduate Program/VRAC

cmake_minimum_required(VERSION 2.6.2)

# Set package properties
project(cppdom)

set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "3")
set(CPACK_PACKAGE_VERSION
	"${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(VERSION_UNDERSCORES "${CPACK_PACKAGE_VERSION_MAJOR}_${CPACK_PACKAGE_VERSION_MINOR}_${CPACK_PACKAGE_VERSION_PATCH}")
###
# Set up options
###

# Define Simple Options


# Define directories
set(BIN_DIR bin)
set(ARCH_DIR lib)
if(WIN32)
	set(LIB_DIR bin)
	set(INCLUDE_DIR include)
else()
	set(LIB_DIR lib)
	set(INCLUDE_DIR include/cppdom-${CPACK_PACKAGE_VERSION})
endif()

###
# End options
###

###
# Perform build configuration of dependencies
###
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(UseBackportedModules)

find_package(Boost)

###
# Build the project
###

include_directories(.)
add_subdirectory(cppdom)

###
# Set packaging options (for CPack)
###

# Choose desired package generators
if(APPLE)
	set(CPACK_GENERATOR DragNDrop)
	set(CPACK_SOURCE_GENERATOR ZIP)
elseif(WIN32)
	set(CPACK_SOURCE_GENERATOR ZIP)
else()
	set(CPACK_SOURCE_GENERATOR TARGZ)
endif()

# Include the packaging system now that we have it all set up
include(CPack)

###
# End Packaging
###
