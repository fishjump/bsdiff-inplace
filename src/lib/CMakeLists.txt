cmake_minimum_required(VERSION 3.22)

add_library(${LIB_DIFF_NAME} STATIC)

target_include_directories(${LIB_DIFF_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(${LIB_DIFF_NAME}
    PRIVATE
        bsdiff.c
        bsearch.c
        qsufsort.c
)

add_library(${LIB_PATCH_NAME} STATIC)

target_include_directories(${LIB_PATCH_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(${LIB_PATCH_NAME}
    PRIVATE
        bspatch.c
)
