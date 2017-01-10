# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

#
# Source
#
set(source_c_files
    ${_UAMQP_ROOT}/src/amqp_definitions.c
    ${_UAMQP_ROOT}/src/amqp_frame_codec.c
    ${_UAMQP_ROOT}/src/amqp_management.c
    ${_UAMQP_ROOT}/src/amqpalloc.c
    ${_UAMQP_ROOT}/src/amqpvalue.c
    ${_UAMQP_ROOT}/src/amqpvalue_to_string.c
    ${_UAMQP_ROOT}/src/cbs.c
    ${_UAMQP_ROOT}/src/connection.c
    ${_UAMQP_ROOT}/src/frame_codec.c
    ${_UAMQP_ROOT}/src/header_detect_io.c
    ${_UAMQP_ROOT}/src/link.c
    ${_UAMQP_ROOT}/src/message.c
    ${_UAMQP_ROOT}/src/message_receiver.c
    ${_UAMQP_ROOT}/src/message_sender.c
    ${_UAMQP_ROOT}/src/messaging.c
    ${_UAMQP_ROOT}/src/sasl_anonymous.c
    ${_UAMQP_ROOT}/src/sasl_frame_codec.c
    ${_UAMQP_ROOT}/src/sasl_mechanism.c
    ${_UAMQP_ROOT}/src/sasl_mssbcbs.c
    ${_UAMQP_ROOT}/src/sasl_plain.c
    ${_UAMQP_ROOT}/src/saslclientio.c
    ${_UAMQP_ROOT}/src/session.c
)

#
# Library target
#
add_library(libuamqp OBJECT ${source_c_files})
set_property(TARGET libuamqp PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(libuamqp 
    PUBLIC ${_IOTSDK_INC}
    PUBLIC ${_UAMQP_INC}
)
