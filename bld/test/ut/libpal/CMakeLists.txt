#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

if (CMAKE_VERSION VERSION_LESS "3.1")
	if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
          # Adding --std=c99 to CMAKE_C_FLAGS breaks compilation of several subprojects!
          # e.g libwebsockets but ${_SHARED_ROOT} needs it!
	  set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
          set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
          # Macro _POSIX_C_SOURCE is needed by ${_SHARED_ROOT}
          set (CMAKE_C_FLAGS "-D_POSIX_C_SOURCE=200112L ${CMAKE_C_FLAGS}")
	endif()
else()
	set (CMAKE_C_STANDARD 99)
    set (CMAKE_CXX_STANDARD 11)
          # Macro _POSIX_C_SOURCE is needed by ${_SHARED_ROOT}
          set (CMAKE_C_FLAGS "-D_POSIX_C_SOURCE=200112L ${CMAKE_C_FLAGS}")
endif()

#
# Unit test includes
#
set(libpal_ut_include
    ${_PROJECT_INC} 
    ${_PROJECT_SRC}/pal 
    ${_IOTSDK_INC} 
    ${_UMOCKC_INC} 
    ${_TRUNNER_INC} 
    ${_CTEST_INC}
)

#
# Misc pal components to test
#
set(_MISC_PAL_UNITS_UNDER_TEST
    new__pal_sk_posix
    pal_err_posix
    pal_ev_kq
    pal_file_posix
    pal_mt_posix
    pal_net_posix
    pal_proc_posix
    new__pal_ws_lws
)

#
# Win pal components to test
#
set(_WIN_PAL_UNITS_UNDER_TEST
    new__pal_sk_win
    pal_err_win
    pal_ev_poll
    pal_file_win
    pal_mt_win
    pal_net_win
    pal_proc_win
    pal_rand_win
    pal_ws_win
)

#
# Linux pal components to test
#
set(_LINUX_PAL_UNITS_UNDER_TEST
    pal_ev_epoll
    pal_net_linux
    pal_proc_linux
    pal_rand_linux
)

#
# Builds a unit tests for each component 
#
foreach(unit ${_MISC_PAL_UNITS_UNDER_TEST} ${_WIN_PAL_UNITS_UNDER_TEST} ${_LINUX_PAL_UNITS_UNDER_TEST}) 
    set(${unit}_ut_test_includes ${libpal_ut_include})
    set(${unit}_ut_test_files ${_PROJECT_TEST}/ut/pal/${unit}_ut.c)
    build_ut(${unit}_ut "proxy_tests")
endforeach()
