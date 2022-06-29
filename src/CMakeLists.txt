cmake_minimum_required(VERSION 3.22)

add_library(${PROJECT_DIFF_NAME} STATIC)

target_include_directories(${PROJECT_DIFF_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(${PROJECT_DIFF_NAME}
    PRIVATE
        bsdiff.c
)

add_library(${PROJECT_PATCH_NAME} STATIC)

target_include_directories(${PROJECT_PATCH_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(${PROJECT_PATCH_NAME}
    PRIVATE
        bspatch.c
)
