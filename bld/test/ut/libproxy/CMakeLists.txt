#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#
# Unit test includes
#
set(libproxy_ut_include
    ${_PROJECT_INC} 
    ${_PROJECT_SRC} 
    ${_UMQTT_INC} 
    ${_IOTSDK_INC} 
    ${_HASHTABLE_ROOT} 
    ${_PARSON_ROOT}
    ${_CMP_ROOT}
    ${_ZLOG_INC}
    ${_UMOCKC_INC} 
    ${_TRUNNER_INC} 
    ${_CTEST_INC}
)

if(WIN32)
set(libproxy_ut_include ${libproxy_ut_include}
    ${_GETOPT_INC}
)
else()
endif()

#
# Proxy components to test
#
set(_PROXY_UNITS_UNDER_TEST
    prx_err
    prx_config
    prx_host
    prx_client
    prx_server
    prx_buffer
    prx_browse
    prx_log
    prx_ns
    prx_sched
    io_browse
    io_codec
    io_mqtt
    io_ref
    io_proto
    io_types
    io_url
    io_queue
    io_token
    io_transport
    io_ws
    pal
    util_misc
    util_signal
    util_string
    util_stream
    util_ringbuf
    util_mem
    xio_sk
    xio_ws
)

compileAsC99()

#
# Builds a unit tests for each component 
#
foreach(unit ${_PROXY_UNITS_UNDER_TEST})
    set(${unit}_ut_test_includes ${libproxy_ut_include})
    set(${unit}_ut_test_files ${_PROJECT_TEST}/ut/${unit}_ut.c)

    build_ut(${unit}_ut "proxy_tests")
endforeach()
