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

# Define directories
set(BIN_DIR bin)
set(ARCH_DIR lib)
set(FLAGPOLL_INFO_DIR share/flagpoll)
if(WIN32)
	set(LIB_DIR bin)
	set(INCLUDE_DIR include)
else()
	set(LIB_DIR lib)
	set(INCLUDE_DIR include/cppdom-${CPACK_PACKAGE_VERSION})
endif()

# Create flagpoll file
set(provides "cppdom")
set(version "${CPACK_PACKAGE_VERSION}")
set(prefix "\${fp_file_cwd}/../..")
set(exec_prefix "\${prefix}")
set(includedir "\${fp_file_cwd}/../../${INCLUDE_DIR}")
set(libdir "\${fp_file_cwd}/../../${LIB_DIR}")
# TODO this is not ideal/complete
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(arch "x86_64")
else()
	set(arch "x86")
endif()
if(CMAKE_COMPILER_IS_GNUCXX)
	set(cppdom_cxxflags "-Wno-deprecated")
	set(include_path_flag "-I")
	set(cppdom_libs "-lcppdom-${VERSION_UNDERSCORES}")
	set(lib_path_flag "-L")
elseif(MSVC)
	set(include_path_flag "/I")
	set(lib_path_flag "/L")
else()
	message(FATAL_ERROR "Compiler not recognized - can't generate values for Flagpoll file")
endif()

configure_file(cppdom.fpc.in "${CMAKE_CURRENT_BINARY_DIR}/cppdom.fpc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cppdom.fpc"
	DESTINATION ${FLAGPOLL_INFO_DIR}
	COMPONENT Development)

###
# Perform build configuration of dependencies
###
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(UseBackportedModules)
include(CTest)

find_package(Boost)

###
# Build the project
###

include_directories(.)
add_subdirectory(cppdom)

add_subdirectory(test)

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
