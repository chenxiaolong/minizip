# NOTE: This is not meant to be a complete build file. It only exists to make it
#       easier to integrate into other CMake projects.

project(minizip)
cmake_minimum_required(VERSION 2.8)

find_package(ZLIB REQUIRED)

include_directories(${ZLIB_INCLUDE_DIRS})

set(SRC_LIST
    ioapi.c
    ioapi_buf.c
    unzip.c
    zip.c
)

if (WIN32)
    list(APPEND SRC_LIST iowin32.c)
elseif(ANDROID)
    list(APPEND SRC_LIST ioandroid.c)
endif()

if(NOT MSVC)
    add_definitions(
        -Wno-empty-body
        -Wno-pedantic
        -Wno-type-limits
        -Wno-unused-parameter
    )
endif()

add_library(minizip STATIC ${SRC_LIST})
#target_link_libraries(minizip ${ZLIB_LIBRARIES})

set_target_properties(
    minizip
    PROPERTIES
    POSITION_INDEPENDENT_CODE 1
)

if(NOT MSVC)
    set_target_properties(
        minizip
        PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED 1
    )
endif()
