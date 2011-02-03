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

	# parsetest
	add_executable(parsetest parsetest.cpp)
	target_link_libraries(parsetest cppdom)
	foreach(testfile game.xml hamlet.xml nodetest.xml test.xml)
		add_test(NAME parsetest-${testfile}
			COMMAND parsetest "${CMAKE_CURRENT_SOURCE_DIR}/${testfile}")
	endforeach()
endif()
