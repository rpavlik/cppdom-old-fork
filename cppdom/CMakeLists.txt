# CMake cross-platform build system
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu>
# http://academic.cleardefinition.com/
# Iowa State University HCI Graduate Program/VRAC

set(API
	config.h
	cppdom.h
	predicates.h
	shared_ptr.h
	SpiritParser.h
	xmlparser.h
	xmltokenizer.h
	version.h)
set(EXT_API
	ext/OptionRepository.h)
set(SOURCES
	cppdom.cpp
	xmlparser.cpp
	xmltokenizer.cpp
	ext/OptionRepository.cpp)

if(BOOST_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	list(APPEND SOURCES SpiritParser.cpp)
endif()

add_definitions(-DCPPDOM_EXPORTS)

set(CPPDOM_LIB cppdom-${CPACK_PACKAGE_VERSION_MAJOR}_${CPACK_PACKAGE_VERSION_MINOR}_${CPACK_PACKAGE_VERSION_PATCH})

add_library(${CPPDOM_LIB} SHARED ${API} ${EXT_API} ${SOURCES})

set_target_properties(${CPPDOM_LIB} PROPERTIES
	PUBLIC_HEADER "${API}"
	COMPILE_DEFINITIONS_DEBUG "CPPDOM_DEBUG")
install(TARGETS ${CPPDOM_LIB}
		RUNTIME DESTINATION ${BIN_DIR}
		LIBRARY DESTINATION	${LIB_DIR}
		ARCHIVE DESTINATION	${ARCH_DIR}
		PUBLIC_HEADER DESTINATION ${INCLUDE_DIR}/cppdom)
install(FILES ${EXT_API}
	DESTINATION ${INCLUDE_DIR}/cppdom/ext)		