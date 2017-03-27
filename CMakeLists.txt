#***************************************************************************
# Copyright:  Matthias Schmieder,
# E-Mail:     schmieder.matthias@gmail.com
# Year:       2016
#***************************************************************************
cmake_minimum_required(VERSION 2.8)


# Set a consistent MACOSX_RPATH default across all CMake versions. 
# When CMake 2.8.12 is required, change this default to 1.
# When CMake 3.0.0 is required, remove this block (see CMP0042).
if(NOT DEFINED CMAKE_MACOSX_RPATH)
  set(CMAKE_MACOSX_RPATH 0)
endif()

project("minizip")

# set cmake debug postfix to d
if(MSVC)
  set(CMAKE_DEBUG_POSTFIX "d")
endif()

# Ensure correct version of zlib is referenced
set(ZLIB_ROOT ${DEF_ZLIB_ROOT} CACHE PATH "Parent directory of zlib installation")
find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
endif()

set(MINIZIP_SRC "crypt.c"
  "ioapi.c"
  "ioapi_buf.c"
  "ioapi_mem.c"
  "minishared.c"
  "unzip.c"
  "zip.c")

set(MINIZIP_PUBLIC_HEADERS "crypt.h"
  "ioapi.h"
  "ioapi_buf.h"
  "ioapi_mem.h"
  "minishared.h"
  "unzip.h"
  "zip.h")

if(WIN32)
  list(APPEND MINIZIP_SRC "iowin32.c")
  list(APPEND MINIZIP_PUBLIC_HEADERS "iowin32.h")
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
elseif(ANDROID)
  list(APPEND MINIZIP_SRC "ioandroid.c")
  list(APPEND MINIZIP_PUBLIC_HEADERS "ioandroid.h")
endif()

if(NOT MSVC)
  add_definitions(
    -D_LARGEFILE64_SOURCE
  )

  add_compile_options(
    -O3
  )

  add_compile_options(
    -Wno-empty-body
    -Wno-format
    -Wno-missing-field-initializers
    -Wno-pedantic
    -Wno-unused-parameter
  )

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
      -Wno-format-pedantic
      -Wno-gnu-statement-expression
      -Wno-gnu-zero-variadic-macro-arguments
      -Wno-language-extension-token
      -Wno-newline-eof
    )
  endif()
endif()

# create minizip library
add_library(minizip-static STATIC ${MINIZIP_SRC} ${MINIZIP_PUBLIC_HEADERS})
add_library(minizip-shared SHARED ${MINIZIP_SRC} ${MINIZIP_PUBLIC_HEADERS})
target_link_libraries(minizip-shared ZLIB::ZLIB)

if(NOT MSVC)
  target_compile_options(minizip-shared PRIVATE -fvisibility=default)
endif()

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

option(USE_AES "enables building of aes library" ON)
if(USE_AES)
  set(AES_SRC 
    aes/aescrypt.c
    aes/aeskey.c
    aes/aestab.c
    aes/fileenc.c
    aes/hmac.c
    aes/prng.c
    aes/pwd2key.c
    aes/sha1.c)

  set(AES_PUBLIC_HEADERS 
    aes/aes.h
    aes/aes_via_ace.h
    aes/aesopt.h
    aes/aestab.h
    aes/brg_endian.h
    aes/brg_types.h
    aes/fileenc.h
    aes/hmac.h
    aes/prng.h
    aes/pwd2key.h
    aes/sha1.h)

  add_library(aes ${AES_SRC} ${AES_PUBLIC_HEADERS})

  add_definitions(-DHAVE_AES)

  target_link_libraries(minizip-static aes)
  target_link_libraries(minizip-shared aes)

#  install(TARGETS aes EXPORT minizip-exports
#    INCLUDES DESTINATION "include"
#    RUNTIME DESTINATION  "bin"
#    LIBRARY DESTINATION  "lib"
#    ARCHIVE DESTINATION  "lib")
#
#  install(FILES ${AES_PUBLIC_HEADERS}
#    DESTINATION "include/minizip/aes")
endif()

#install(TARGETS minizip-static minizip-shared EXPORT minizip-exports
#  INCLUDES DESTINATION "include"
#  RUNTIME DESTINATION "bin"
#  LIBRARY DESTINATION "lib"
#  ARCHIVE DESTINATION "lib")
#
#install(EXPORT minizip-exports
#        DESTINATION "cmake"
#        NAMESPACE "MINIZIP::")
#
#install(FILES ${MINIZIP_PUBLIC_HEADERS}
#  DESTINATION "include/minizip")

option (BUILD_TEST "enabled building of executables minizip and miniunz. Requires ZLIB!" OFF)
if(BUILD_TEST)
  add_executable(miniunz_exec miniunz.c)
  target_link_libraries(miniunz_exec minizip)

  add_executable(minizip_exec minizip.c)
  target_link_libraries(minizip_exec minizip)

#  install(TARGETS miniunz_exec minizip_exec
#          RUNTIME DESTINATION "bin")
endif()

# DualBootPatcher-specific
if(DEFINED MBP_BUILD_TARGET
    AND (${MBP_BUILD_TARGET} STREQUAL android-app)
        OR (${MBP_BUILD_TARGET} STREQUAL desktop))
  install(
    TARGETS minizip-shared
    LIBRARY DESTINATION ${LIB_INSTALL_DIR} COMPONENT Libraries
    RUNTIME DESTINATION ${LIB_INSTALL_DIR} COMPONENT Libraries
    #ARCHIVE DESTINATION ${LIB_INSTALL_DIR} COMPONENT Libraries
  )
endif()
