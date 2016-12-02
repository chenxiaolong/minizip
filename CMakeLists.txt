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

add_library(minizip-static STATIC ${SRC_LIST})
add_library(minizip-shared SHARED ${SRC_LIST})

target_compile_options(minizip-shared PRIVATE -fvisibility=default)

target_link_libraries(minizip-shared ${ZLIB_LIBRARIES})

set_target_properties(
    minizip-static minizip-shared
    PROPERTIES
    POSITION_INDEPENDENT_CODE 1
)
set_target_properties(
    minizip-shared
    PROPERTIES
    OUTPUT_NAME minizip
)

if(NOT MSVC)
    set_target_properties(
        minizip-static minizip-shared
        PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED 1
    )
endif()

# Install minizip
if(DEFINED MBP_BUILD_TARGET
        AND (${MBP_BUILD_TARGET} STREQUAL android-app
                OR ${MBP_BUILD_TARGET} STREQUAL desktop))
    install(
        TARGETS minizip-shared
        LIBRARY DESTINATION ${LIB_INSTALL_DIR} COMPONENT Libraries
        RUNTIME DESTINATION ${LIB_INSTALL_DIR} COMPONENT Libraries
        #ARCHIVE DESTINATION ${LIB_INSTALL_DIR} COMPONENT Libraries
    )
endif()
