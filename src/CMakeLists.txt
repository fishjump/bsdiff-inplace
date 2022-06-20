cmake_minimum_required(VERSION 3.22)

add_library(${CMAKE_PROJECT_NAME} STATIC)

target_include_directories(${CMAKE_PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        dummy.c
)
