# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

if (CMAKE_VERSION VERSION_LESS "3.1")
	if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
          # Adding --std=c99 to CMAKE_C_FLAGS breaks compilation of several subprojects!
          # e.g libwebsockets but ${_SHARED_ROOT} needs it!
	  set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
          set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
	endif()
else()
	set (CMAKE_C_STANDARD 99)
    set (CMAKE_CXX_STANDARD 11)
endif()

#
# Source
#
set(source_c_files
    ${_UMQTT_ROOT}/src/mqtt_client.c
    ${_UMQTT_ROOT}/src/mqtt_codec.c
    ${_UMQTT_ROOT}/src/mqtt_message.c
)

#
# Library target
#
add_library(libumqtt OBJECT ${source_c_files})
set_property(TARGET libumqtt PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(libumqtt 
    PUBLIC ${_IOTSDK_INC}
    PUBLIC ${_UMQTT_INC}
)
