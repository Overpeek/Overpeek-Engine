# include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(test_exe test_name)
	add_executable(${test_name} "${test_name}/main.cpp")
	set_target_properties(${test_name} PROPERTIES FOLDER "engine/tests")
	target_link_libraries(${test_name} PRIVATE engine)
	target_include_directories(${test_name} PRIVATE ${engine_INCLUDE_DIRS})
	add_test("${test_name}_TEST" ${test_name} --ctest)
endfunction()

test_exe("entities")
test_exe("experimental")
test_exe("game-of-life")
test_exe("guis")
test_exe("hello-world")
test_exe("networking")
test_exe("polygon")
test_exe("rendering")
test_exe("text")

if(OE_BUILD_MODE EQUAL 2)
target_compile_definitions(engine PRIVATE BUILD_VULKAN)
	test_exe("vulkan")
endif()