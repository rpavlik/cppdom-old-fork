if(BUILD_TESTING)
	include_directories(. ${CPPUNIT_INCLUDE_DIRS})
	set(SOURCES 
		runner.cpp
		Suites.h
		testHelpers.h
		extensions/MetricRegistry.h
		TestCases/ErrorTest.cpp
		TestCases/ErrorTest.h
		TestCases/NodeTest.cpp
		TestCases/NodeTest.h
		TestCases/OptionRepositoryTest.cpp
		TestCases/OptionRepositoryTest.h
		TestCases/ParseTest.cpp
		TestCases/ParseTest.h
		TestCases/PredTest.cpp
		TestCases/PredTest.h
		TestCases/TestData.h)
	if(BOOST_FOUND)
		list(APPEND SOURCES
			TestCases/SpiritTest.cpp
			TestCases/SpiritTest.h)
	endif()
	add_executable(runner ${SOURCES})
	target_link_libraries(runner cppdom ${CPPUNIT_LIBRARIES})
	add_test(NAME testsuite-noninteractive
		COMMAND
		${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_SOURCE_DIR}" "$<TARGET_FILE:runner>")
	#add_test(NAME testsuite-metric
	#	COMMAND
	#	${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_SOURCE_DIR}" "$<TARGET_FILE:runner>" metric)
endif()
