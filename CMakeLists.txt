###########################################
# main CMake of jslang
###########################################
cmake_minimum_required(VERSION 2.4 FATAL_ERROR)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(jslang C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

###########################################
# find llvm first
###########################################
find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config PATHS "C:/MyProject/llvm-3.4.2/bin/Debug" DOC "llvm-config executable")

execute_process(
	COMMAND ${LLVM_CONFIG_EXECUTABLE} --cppflags
	OUTPUT_VARIABLE LLVM_CFLAGS
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
	OUTPUT_VARIABLE LLVM_LFLAGS
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs all
	OUTPUT_VARIABLE LLVM_ALL_LIBS
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
STRING(REGEX REPLACE "-L([^ ]*)(.*)" "\1" LLVM_LIB_PATH ${LLVM_LFLAGS})

###########################################
# common settings and options
###########################################
set(LUA_VERSION_MAJOR 5)
set(LUA_VERSION_MINOR 1)
set(LUA_VERSION_PATCH 4)
set(LUA_VERSION
				"${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}.${LUA_VERSION_PATCH}")
set(LUA_SOVERSION
				"${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}")

set(COMMON_CFLAGS)
set(COMMON_LDFLAGS)
set(LIBS)

#
# llvm-lua options.
#
option(LUA_CPP_SUPPORT "Enable c++ support" OFF)
set(LLVM_CC "clang" CACHE STRING "Command to compile C-code to LLVM bitcode (clang or llvm-gcc)")
set(LLVM_LD "llvm-ld" CACHE STRING "Command to link LLVM bitcode files into one bitcode file.")

#
# standard flags to use for each build type.
#
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe -Wall -Wshadow -W -std=gnu89 ")
	set(CMAKE_C_FLAGS_RELEASE        "${CMAKE_C_FLAGS_RELEASE}     -O2")
	set(CMAKE_C_FLAGS_DEBUG          "${CMAKE_C_FLAGS_DEBUG}       -O0 -g")
	set(CMAKE_C_FLAGS_PROFILE        "${CMAKE_C_FLAGS_PROFILE}     -O1 -g")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_WITHDEBINFO} -O2 -g")
endif(CMAKE_COMPILER_IS_GNUCC)

#
# For uninstall (needs cmake_uninstall.cmake.in in the top-level directory)
#

###########################################
# platform settings and options
###########################################
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
elseif(APPLE)
elseif(CYGWIN)
elseif(UNIX)
elseif(WIN32)
	set(LLVM_LFLAGS "/LIBPATH:\"${LLVM_LIB_PATH}Debug\"")
	message("LLVM_LFLAGS: " ${LLVM_LFLAGS})
	STRING(REGEX REPLACE "-l([^ ]*)" "\1" LLVM_ALL_LIBS ${LLVM_ALL_LIBS})
	message("LLVM_ALL_LIBS: " ${LLVM_ALL_LIBS})
	STRING(REGEX REPLACE " " ";" LLVM_ALL_LIBS ${LLVM_ALL_LIBS})
	set(PTHREAD_LIBS ws2_32)
else(APPLE)
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

# Workaround for MSVS10 to avoid the Dialog Hell
# FIXME: This could be removed with future version of CMake.
if(MSVC_VERSION EQUAL 1600 )
  set(JSLANG_SLN_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/jslang.sln")
  if( EXISTS "${JSLANG_SLN_FILENAME}" )
    file(APPEND "${JSLANG_SLN_FILENAME}" "\n# This should be regenerated!\n")
  endif()
endif()

set(BUG_REPORT_URL "https://github.com/eddid/jslang/issues" CACHE STRING
  "Default URL where bug reports are to be submitted.")

###########################################
# macros
###########################################
macro(add_target_properties _target _name)
	set(_properties)
	foreach(_prop ${ARGN})
		set(_properties "${_properties} ${_prop}")
	endforeach(_prop)
	get_target_property(_old_properties ${_target} ${_name})
	if(NOT _old_properties)
		# in case it's NOTFOUND
		set(_old_properties)
	endif(NOT _old_properties)
	set_target_properties(${_target} PROPERTIES ${_name} "${_old_properties} ${_properties}")
endmacro(add_target_properties)

###########################################
# make all project in one CMakefile
###########################################
find_library(pcre_library pcre ${JSLANG_PATH_TO_LLVM_BUILD}/bin/Debug)
find_library(pthread_library pthread ${JSLANG_PATH_TO_LLVM_BUILD}/bin/Debug)
add_definitions(-DPCRE_STATIC)
include_directories(
  ejs_runtime
  include/jslang/AST
  include/jslang/Basic
  include/jslang/CodeGen
  include/jslang/Lex
  include/jslang/Parse
  third_party/pcre
  third_party/pthreads
  )

set(PCRE_LIB_SRC
  third_party/pcre/pcre_byte_order.c
  third_party/pcre/pcre_chartables.c
  third_party/pcre/pcre_compile.c
  third_party/pcre/pcre_config.c
  third_party/pcre/pcre_dfa_exec.c
  third_party/pcre/pcre_exec.c
  third_party/pcre/pcre_fullinfo.c
  third_party/pcre/pcre_get.c
  third_party/pcre/pcre_globals.c
  third_party/pcre/pcre_jit_compile.c
  third_party/pcre/pcre_maketables.c
  third_party/pcre/pcre_newline.c
  third_party/pcre/pcre_ord2utf8.c
  third_party/pcre/pcre_refcount.c
  third_party/pcre/pcre_string_utils.c
  third_party/pcre/pcre_study.c
  third_party/pcre/pcre_tables.c
  third_party/pcre/pcre_ucd.c
  third_party/pcre/pcre_valid_utf8.c
  third_party/pcre/pcre_version.c
  third_party/pcre/pcre_xclass.c
  )

set(PTHREAD_LIB_SRC
  third_party/pthreads/pthread.c
  )
set(JSLANG_SRC
  lib/AST/Nodes.cpp
  lib/CodeGen/CodeGen.cpp
  lib/CodeGen/CodeGenJSObject.cpp
  lib/CodeGen/CodeGenJSValue.cpp
  lib/CodeGen/CodeGenNatives.cpp
  lib/Lex/Lexer.cpp
  lib/Parse/Parser.cpp
  tools/driver/jsextern.cpp
  tools/driver/jslang.cpp
  ejs_runtime/allinone_for_link.cpp
  )

set(EJS_RUNTIME_SRC
  tools/driver/ejs_runtime.cpp
  tools/driver/jsextern.cpp
  )

add_library(pcre STATIC ${PCRE_LIB_SRC})
add_target_properties(pcre COMPILE_FLAGS "${COMMON_CFLAGS} -DHAVE_CONFIG_H -DSUPPORT_UTF8")
add_target_properties(pcre LINK_FLAGS "${LD_FLAGS} ${COMMON_LDFLAGS}")
target_link_libraries(pcre ${LIBS})

add_library(pthread SHARED ${PTHREAD_LIB_SRC})
add_target_properties(pthread COMPILE_FLAGS "${COMMON_CFLAGS} -DPTW32_BUILD")
add_target_properties(pthread LINK_FLAGS "${LD_FLAGS} ${COMMON_LDFLAGS}")
target_link_libraries(pthread ${LIBS} ${PTHREAD_LIBS})

add_executable(ejs_runtime ${EJS_RUNTIME_SRC})
add_target_properties(ejs_runtime COMPILE_FLAGS "${COMMON_CFLAGS} ${LLVM_CFLAGS}")
add_target_properties(ejs_runtime LINK_FLAGS "${LD_FLAGS} ${COMMON_LDFLAGS} ${LLVM_LFLAGS}")
target_link_libraries(ejs_runtime pcre pthread ${LLVM_ALL_LIBS})

add_executable(jslang ${JSLANG_SRC})
add_target_properties(jslang COMPILE_FLAGS "${COMMON_CFLAGS} ${LLVM_CFLAGS}")
add_target_properties(jslang LINK_FLAGS "${LD_FLAGS} ${COMMON_LDFLAGS} ${LLVM_LFLAGS}")
target_link_libraries(jslang pcre pthread ${LLVM_ALL_LIBS})