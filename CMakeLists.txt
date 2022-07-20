cmake_minimum_required(VERSION 3.22)

project(bsdiff)

include(cmake/git-submodule-update.cmake)
GitSubmoduleUpdate()

# Generate the compile_commands.json for ide autocompletion.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(LIB_BZIP_NAME bz2)
set(LIB_DIFF_NAME bsdiff)
set(LIB_PATCH_NAME bspatch)
set(BIN_DIFF_NAME bsdiff_bin)
set(BIN_PATCH_NAME bspatch_bin)
set(PROJECT_TEST_NAME bsdiff_test)

add_subdirectory(lib)
add_subdirectory(src)

find_package(GTest)
if(GTEST_FOUND)
  list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
  enable_testing()
  add_subdirectory(test)
else()
  message(WARNING "GTest not found")
endif()
