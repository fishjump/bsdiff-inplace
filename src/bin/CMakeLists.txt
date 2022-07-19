cmake_minimum_required(VERSION 3.22)

add_executable(${BIN_DIFF_NAME})

target_include_directories(${BIN_DIFF_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/lib/bzip2
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${BIN_DIFF_NAME}
    PRIVATE
        ${LIB_BZIP_NAME}
        ${LIB_DIFF_NAME}
)

target_sources(${BIN_DIFF_NAME}
    PRIVATE
        bsdiff.c
)

add_executable(${BIN_PATCH_NAME})

target_include_directories(${BIN_PATCH_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/lib/bzip2
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${BIN_PATCH_NAME}
    PRIVATE
        ${LIB_BZIP_NAME}
        ${LIB_PATCH_NAME}
)

target_sources(${BIN_PATCH_NAME}
    PRIVATE
        bspatch.c
)
