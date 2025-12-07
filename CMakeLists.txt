
if(NOT DEFINED ENV{COMMONLIB_SSE_FOLDER})
  message(FATAL_ERROR "Missing COMMONLIB_SSE_FOLDER environment variable")
endif()

include(cmake/headerlist.cmake)
include(cmake/sourcelist.cmake)
include(cmake/lib/copyOutputs.cmake)
include(cmake/lib/automaticGameFolderOutput.cmake)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# It's recommended to set a minimum CMake version.
# If you use CMake features from higher versions, update this to match.
cmake_minimum_required(VERSION 3.21)
# Set your project name. This will be the name of your SKSE .dll file.
set(AUTHOR_NAME "Quantumyilmaz")
set(PRODUCT_NAME "PersistentFavorites")
project(${PRODUCT_NAME} VERSION 1.0.2.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY)

set(RESOURCE_FILE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
set(RESOURCE_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version.res)

add_custom_command(
    OUTPUT ${RESOURCE_OUTPUT}
    COMMAND rc /fo ${RESOURCE_OUTPUT} ${RESOURCE_FILE}
    DEPENDS ${RESOURCE_FILE}
)

# Add a custom target to ensure the resource file is generated
add_custom_target(Resource ALL DEPENDS ${RESOURCE_OUTPUT})
include(FetchContent)

set(ENABLE_SKYRIM_SE ON CACHE BOOL " " FORCE)
set(ENABLE_SKYRIM_AE ON CACHE BOOL " " FORCE)
set(ENABLE_SKYRIM_VR ON CACHE BOOL " " FORCE)
set(BUILD_TESTS OFF CACHE BOOL " " FORCE)

add_subdirectory("$ENV{COMMONLIB_SSE_FOLDER}" ${CMAKE_CURRENT_BINARY_DIR}/_deps/clib-build)
get_target_property(COMMONLIB_SRC_DIR CommonLibSSE SOURCE_DIR)

find_path(CLIB_UTIL_INCLUDE_DIRS "ClibUtil/utils.hpp")
find_path(ClibUtilsQTR_INCLUDE_DIRS "ClibUtilsQTR/utils.hpp")

file(
  GLOB_RECURSE
  sources
  ${CMAKE_CURRENT_BINARY_DIR}/version.rc
  ${sources}
)

source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR}
  FILES ${headers} ${sources}
)

set(BUILD_TESTS OFF)
include(${COMMONLIB_SRC_DIR}/cmake/CommonLibSSE.cmake)

add_commonlibsse_plugin(
  ${PROJECT_NAME}
  AUTHOR AUTHOR_NAME
  SOURCES ${headers} ${sources}
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23) # <--- use C++23 standard
target_precompile_headers(${PROJECT_NAME} PRIVATE include/PCH.h) # <--- PCH.h is required!
target_include_directories(
	${PROJECT_NAME}
	PRIVATE
	${CLIB_UTIL_INCLUDE_DIRS}
    ${ClibUtilsQTR_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (MSVC)
  # Good default
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)

  # If you really want MSVC to warn on truly unused (internal-linkage) functions (C4505),
  # enable /Wall. It is very noisy, so consider disabling a few chatty warnings.
  #target_compile_options(${PROJECT_NAME} PRIVATE /Wall
  #   /wd4514 /wd4710 /wd4711 /wd4820 /wd5045)

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic
    -Wunused-function
    -Wunused-member-function) # catches unused class methods (Clang)

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic
    -Wunused-function) # GCC does not warn on unused member funcs
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE IS_HOST_PLUGIN)

set(wildlander_output false)
set(steam_owrt_output false)
set(steam_mods_output true)
set(lorerim_mods_output false)
set(lorerim3_mods_output true)
automaticGameFolderOutput(wildlander_output steam_owrt_output steam_mods_output lorerim_mods_output lorerim3_mods_output)
