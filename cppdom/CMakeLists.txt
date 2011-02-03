# CMake-based build system for cppdom
# 2009-2011 Ryan Pavlik <rpavlik@iastate.edu>
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

add_library(cppdom SHARED ${API} ${EXT_API} ${SOURCES})

set_target_properties(cppdom PROPERTIES
	PUBLIC_HEADER "${API}"
	COMPILE_DEFINITIONS_DEBUG "CPPDOM_DEBUG")
install(TARGETS cppdom
		RUNTIME DESTINATION ${BIN_DIR}
		LIBRARY DESTINATION	${LIB_DIR}
		ARCHIVE DESTINATION	${ARCH_DIR}
		PUBLIC_HEADER DESTINATION ${INCLUDE_DIR}/cppdom)
install(FILES ${EXT_API}
	DESTINATION ${INCLUDE_DIR}/cppdom/ext)		