find_library(DLMALLOC_LIBRARY_x64
    NAMES dlmalloc
    HINTS "${CMAKE_SOURCE_DIR}/lib/x64")

find_library(DLMALLOC_LIBRARY_x86
    NAMES dlmalloc
    HINTS "${CMAKE_SOURCE_DIR}/lib/x86")
