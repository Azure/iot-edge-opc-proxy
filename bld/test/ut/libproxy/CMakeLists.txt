#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

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
)

if(WIN32)
set(libproxy_ut_include ${libproxy_ut_include}
	${_GETOPT_ROOT}
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
	set(libproxy_ut_files ${libproxy_ut_files} ${_PROJECT_TEST}/ut/${unit}_ut.c)

	set(${unit}_ut_test_includes ${libproxy_ut_include})
	set(${unit}_ut_test_files ${_PROJECT_TEST}/ut/${unit}_ut.c)

	build_ut(${unit}_ut "proxy_tests")
endforeach()

#
# On windows, build vstest dll
#
if(WIN32)
    link_directories($ENV{VCInstallDir}UnitTest/lib)
    add_library(libproxy_ut SHARED 
		${libproxy_ut_files}
    )
    set_target_properties(libproxy_ut PROPERTIES FOLDER "proxy_tests") 

    target_compile_definitions(libproxy_ut PUBLIC -DCPP_UNITTEST)
    target_compile_options(libproxy_ut PUBLIC /TP /EHsc)

    target_include_directories(libproxy_ut PUBLIC $ENV{VCInstallDir}UnitTest/include ${libproxy_ut_include})
	target_link_libraries(libproxy_ut libtest)
endif()

