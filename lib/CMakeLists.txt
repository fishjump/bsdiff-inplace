cmake_minimum_required(VERSION 3.22)

add_library(${LIB_BZIP_NAME} STATIC)

target_compile_definitions(${LIB_BZIP_NAME} 
    PRIVATE 
        _FILE_OFFSET_BITS=64
)

target_include_directories(${LIB_BZIP_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/bzip2
)

target_sources(${LIB_BZIP_NAME}
    PRIVATE
        bzip2/blocksort.c
        bzip2/bzlib.c
        bzip2/compress.c
        bzip2/crctable.c
        bzip2/decompress.c
        bzip2/huffman.c
        bzip2/randtable.c
)
