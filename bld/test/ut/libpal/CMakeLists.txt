#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#
# Unit test includes
#
set(libpal_ut_include
    ${_PROJECT_INC} 
    ${_PROJECT_SRC}/pal 
    ${_IOTSDK_INC} 
    ${UMOCK_C_INC_FOLDER} 
    ${TESTRUNNERSWITCHER_INC_FOLDER} 
    ${CTEST_INC_FOLDER}
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
