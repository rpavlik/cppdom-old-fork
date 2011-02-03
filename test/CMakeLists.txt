find_package(cppunit)
if(CPPUNIT_FOUND)
	add_subdirectory(suite)
endif()

if(BUILD_TESTING)
	include_directories(.)

	# maketree
	add_executable(maketree maketree.cpp)
	target_link_libraries(maketree cppdom)
	add_test(NAME maketree
		COMMAND maketree)

	# nodetest
	include(CopyResourcesToBuildTree)
	add_executable(nodetest nodetest.cpp game.xml)
	set_target_properties(nodetest PROPERTIES RESOURCE "game.xml")
	target_link_libraries(nodetest cppdom)
	copy_resources_to_build_tree(nodetest)
	add_test(NAME nodetest
		COMMAND nodetest)
	set_tests_properties(nodetest PROPERTIES
		FAIL_REGULAR_EXPRESSION "[eE]rror"
		PASS_REGULAR_EXPRESSION "in name: bokbokbok")

	# parsetest
	add_executable(parsetest parsetest.cpp)
	target_link_libraries(parsetest cppdom)
	foreach(testfile game.xml hamlet.xml nodetest.xml test.xml)
		add_test(NAME parsetest-${testfile}
			COMMAND parsetest "${CMAKE_CURRENT_SOURCE_DIR}/${testfile}")
		set_tests_properties(parsetest-${testfile} PROPERTIES
			FAIL_REGULAR_EXPRESSION "[eE]rror")
	endforeach()
endif()
