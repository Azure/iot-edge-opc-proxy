#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

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
# pal components to test
#
set(_PAL_UNITS_UNDER_TEST
    pal_cred_gnome
    pal_cred_stub
    pal_cred_win
    pal_err_posix
    pal_err_win
    pal_ev_epoll
    pal_ev_kq
    pal_ev_poll
    pal_ev_win
    pal_file_posix
    pal_file_win
    pal_mt_posix
    pal_mt_win
    pal_net_linux
    pal_net_posix
    pal_net_win
    pal_proc_linux
    pal_proc_posix
    pal_proc_win
    pal_rand_linux
    pal_rand_openssl
    pal_rand_win
    pal_sd_avahi
    pal_sd_mdns
    pal_sd_stub
    pal_sk_posix
    pal_sk_win
    pal_time_posix
    pal_time_win
    pal_ws_lws
    pal_ws_stub
    pal_ws_win
)

compileAsC99()

#
# Builds a unit tests for each component 
#
foreach(unit ${_PAL_UNITS_UNDER_TEST}) 
    set(${unit}_ut_test_includes ${libpal_ut_include})
    set(${unit}_ut_test_files ${_PROJECT_TEST}/ut/pal/${unit}_ut.c)
    build_ut(${unit}_ut "proxy_tests")
endforeach()
