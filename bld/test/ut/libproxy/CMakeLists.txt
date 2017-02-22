#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

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
# Unit test includes
#
set(libproxy_ut_include
    ${_PROJECT_INC} 
    ${_PROJECT_SRC} 
    ${_UAMQP_INC} 
    ${_UMQTT_INC} 
    ${_IOTSDK_INC} 
    ${_HASHTABLE_ROOT} 
    ${_PARSON_ROOT}
    ${_CMP_ROOT}
    ${_ZLOG_INC}
    ${_UMOCKC_INC} 
    ${_TRUNNER_INC} 
    ${_CTEST_INC}
    ${CMAKE_GENERATED_INC}
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
    new__prx_host
    new__prx_client
    new__prx_server
    new__prx_buffer
    new__prx_ns
    new__prx_sched
    new__io_codec
    new__io_mqtt
    io_ref
    io_proto
    io_types
    io_url
    new__io_queue
    new__io_token
    new__io_transport
    new__io_ws
    pal
    util_handle
    util_misc
    util_signal
    util_string
    new__util_stream
    new__util_log
    new__util_mem
    new__xio_sk
    new__xio_ws
)

#
# Builds a unit tests for each component 
#
foreach(unit ${_PROXY_UNITS_UNDER_TEST})
    set(${unit}_ut_test_includes ${libproxy_ut_include})
    set(${unit}_ut_test_files ${_PROJECT_TEST}/ut/${unit}_ut.c)

    build_ut(${unit}_ut "proxy_tests")
endforeach()
