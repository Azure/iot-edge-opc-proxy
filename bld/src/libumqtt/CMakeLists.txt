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

#
# Library target
#
add_library(libumqtt OBJECT ${source_c_files})
set_property(TARGET libumqtt PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(libumqtt 
    PUBLIC ${_IOTSDK_INC}
    PUBLIC ${_UMQTT_INC}
)
