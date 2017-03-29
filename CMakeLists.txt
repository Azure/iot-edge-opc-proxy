#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#
# Cmake project settings
#
cmake_minimum_required(VERSION 2.8.11)

project(azure-iot-proxy)

include(CMakeToolsHelpers OPTIONAL)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(_PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "Root" FORCE)

enable_testing()

#
# output paths
#
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Add rule to generate TAGS for emacs and vi
if(MSVC)
else()
  add_custom_target(TAGS
    COMMAND etags ${CMAKE_SOURCE_DIR}/*/*.[hc] )
  add_custom_target(tags
    COMMAND ctags ${CMAKE_SOURCE_DIR}/*/*.[hc] )
endif()

#
# Sub-projects
#
add_subdirectory(bld)
