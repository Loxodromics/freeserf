cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

include(GoogleTest)

if(WIN32)
  add_definitions(/D_CRT_SECURE_NO_WARNINGS)
endif()

set(TEST_MAP_SOURCES test_map.cc)
add_executable(test_map ${TEST_MAP_SOURCES})
target_check_style(test_map)
set_property(TARGET test_map PROPERTY FOLDER "Tests")
target_link_libraries(test_map game tools GTest::gtest GTest::gtest_main ${CMAKE_THREAD_LIBS_INIT})
gtest_add_tests(TARGET test_map
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.."
                TEST_LIST test_list)
foreach(test IN LISTS test_list)
  set_tests_properties(${test} PROPERTIES ENVIRONMENT "GTEST_OUTPUT=xml:${PROJECT_BINARY_DIR}/${test}.xml")
endforeach(test)

set(TEST_SAVE_GAME_SOURCES test_save_game.cc)
add_executable(test_save_game ${TEST_SAVE_GAME_SOURCES})
target_check_style(test_save_game)
set_property(TARGET test_save_game PROPERTY FOLDER "Tests")
target_link_libraries(test_save_game game tools GTest::gtest GTest::gtest_main ${CMAKE_THREAD_LIBS_INIT})
gtest_add_tests(TARGET test_save_game
                TEST_LIST test_list)
foreach(test IN LISTS test_list)
  set_tests_properties(${test} PROPERTIES ENVIRONMENT "GTEST_OUTPUT=xml:${PROJECT_BINARY_DIR}/${test}.xml")
endforeach(test)

set(TEST_MAP_GEOMETRY_SOURCES test_map_geometry.cc)
add_executable(test_map_geometry ${TEST_MAP_GEOMETRY_SOURCES})
target_check_style(test_map_geometry)
set_property(TARGET test_map_geometry PROPERTY FOLDER "Tests")
target_link_libraries(test_map_geometry game tools GTest::gtest GTest::gtest_main ${CMAKE_THREAD_LIBS_INIT})
gtest_add_tests(TARGET test_map_geometry
                TEST_LIST test_list)
foreach(test IN LISTS test_list)
  set_tests_properties(${test} PROPERTIES ENVIRONMENT "GTEST_OUTPUT=xml:${PROJECT_BINARY_DIR}/${test}.xml")
endforeach(test)
