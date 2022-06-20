cmake_minimum_required(VERSION 3.22)

project(diff)

# Generate the compile_commands.json for ide autocompletion.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(PROJECT_TEST_NAME ${CMAKE_PROJECT_NAME}_test)

add_subdirectory(src)

find_package(GTest)
if(GTEST_FOUND)
  list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
  enable_testing()
  add_subdirectory(test)
else()
  message(WARNING "GTest not found")
endif()
