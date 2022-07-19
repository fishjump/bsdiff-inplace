cmake_minimum_required(VERSION 3.22)

add_library(fastlz STATIC)

target_include_directories(fastlz
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/FastLZ
)

target_sources(fastlz
    PRIVATE
        FastLZ/fastlz.c
)