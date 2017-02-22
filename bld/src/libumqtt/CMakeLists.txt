# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

#
# Source
#
set(source_c_files
    ${_UMQTT_ROOT}/src/mqtt_client.c
    ${_UMQTT_ROOT}/src/mqtt_codec.c
    ${_UMQTT_ROOT}/src/mqtt_message.c
)

compileAsC99()

#
# Library target
#
add_library(libumqtt OBJECT ${source_c_files})
set_property(TARGET libumqtt PROPERTY POSITION_INDEPENDENT_CODE ON)

if(WIN32)
else()
	target_compile_definitions(libumqtt PRIVATE _POSIX_C_SOURCE=200112L)
endif()

target_include_directories(libumqtt 
    PUBLIC ${_IOTSDK_INC}
    PUBLIC ${_UMQTT_INC}
)
