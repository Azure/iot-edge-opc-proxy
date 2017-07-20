# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

#
# proxy platform independent components
#
set(_PROXY_COMPONENTS
    prx_err
    prx_host
    prx_config
    prx_client
    prx_server
    prx_sched
    prx_log
    prx_module
    prx_buffer
    prx_browse
    prx_ns
    io_codec
    io_cs
    io_mqtt
    io_proto
    io_queue
    io_ref
    io_token
    io_transport
    io_types
    io_browse
    io_url
    io_ws
    pal
    util_mem
    util_misc
    util_ringbuf
    util_signal
    util_stream
    util_string
    xio_sk
    xio_ws
)

#
# proxy pal layer composition
#
if(WIN32)
    set(_PAL_COMPONENTS ${_PAL_COMPONENTS}
        pal_err_win
        pal_cred_win
        pal_file_win
        pal_main_win
        pal_mt_win
        pal_proc_win
        pal_net_posix
        pal_net_win
        pal_rand_win
        pal_sk_win
        pal_time_win
    )
    if (NOT use_lws)
        set(_PAL_COMPONENTS ${_PAL_COMPONENTS} pal_ws_win)
    endif()

    if(use_dnssd OR prefer_dnssd_embedded_api)
        set(_PAL_COMPONENTS ${_PAL_COMPONENTS} pal_sd_mdns pal_ev_poll)
        set(_PAL_DEP_LIBS ${_PAL_DEP_LIBS} dnssd)
        set(_PAL_DEP_INC ${_PAL_DEP_INC} PUBLIC ${_DNSSD_ROOT}/src)
        message(STATUS "  linking with dnssd")
    else()
        set(_PAL_COMPONENTS ${_PAL_COMPONENTS} pal_sd_stub)
    endif()
else()

    include(CheckIncludeFile)
    include(FindPkgConfig)

    set(_PAL_COMPONENTS ${_PAL_COMPONENTS}
        pal_err_posix
        pal_file_posix
        pal_mt_posix
        pal_net_posix
        pal_sk_posix
        pal_time_posix
    )

    check_include_file("linux/random.h" HAVE_LINUX_RANDOM_H)
    if(HAVE_LINUX_RANDOM_H)
        set(_PAL_COMPONENTS ${_PAL_COMPONENTS} pal_rand_linux)
        message(STATUS "  linking with urandom rand")
    else()
        set(use_openssl 1)
        set(_PAL_COMPONENTS ${_PAL_COMPONENTS} pal_rand_openssl)
        message(STATUS "  linking with openssl rand")
    endif()

    if(CMAKE_CROSSCOMPILING)
        # As mentioned at https://cmake.org/Wiki/CMake_Cross_Compiling the
        # pkg-config tool can not be used by cmake while cross compiling.
        set(_PAL_COMPONENTS ${_PAL_COMPONENTS} pal_cred_stub)
    else()
        pkg_check_modules(LIBSECRET libsecret-1)
        if(LIBSECRET_FOUND)
            set(_PAL_COMPONENTS ${_PAL_COMPONENTS} pal_cred_gnome)
            set(_PAL_DEP_LIBS ${_PAL_DEP_LIBS} ${LIBSECRET_LIBRARIES})
            set(_PAL_DEP_INC ${_PAL_DEP_INC} ${LIBSECRET_INCLUDE_DIRS})
            set(_PAL_DEP_DEFS ${_PAL_DEP_DEFS} __HAVE_LIBSECRET=1)
            message(STATUS "  linking with libsecret-1")
        else()
            pkg_check_modules(LIBGNOMEKEYRING gnome-keyring-1)
            if(LIBGNOMEKEYRING_FOUND)
                set(_PAL_COMPONENTS ${_PAL_COMPONENTS} pal_cred_gnome)
                set(_PAL_DEP_LIBS ${_PAL_DEP_LIBS} ${LIBGNOMEKEYRING_LIBRARIES})
                set(_PAL_DEP_INC ${_PAL_DEP_INC} ${LIBGNOMEKEYRING_INCLUDE_DIRS})
                message(STATUS "  linking with gnome-keyring")
            else()
                set(_PAL_COMPONENTS ${_PAL_COMPONENTS} pal_cred_stub)
                message(WARNING "no secret store - install libsecret-1!")
            endif()
        endif()
    endif()

    if(use_dnssd OR prefer_dnssd_embedded_api)
        set (use_dnssd 1)
        if(CMAKE_CROSSCOMPILING)
            message(STATUS "  forced linking with avahi for cross compiling")
            set(LIBAVAHI_LIBRARIES avahi-client avahi-common)
            set(LIBAVAHI_FOUND 1)

            set(LIBAVAHICORE_LIBRARIES avahi-client avahi-common)
            set(LIBAVAHICORE_FOUND 1)
        else()
            pkg_check_modules(LIBAVAHICORE avahi-core)
            pkg_check_modules(LIBAVAHI avahi-client)

            if (NOT LIBAVAHI_FOUND)
                if (LIBAVAHICORE_FOUND)
                    set (prefer_dnssd_embedded_api 1)
                elseif (NOT LIBAVAHI_FOUND)
                    message(WARNING "no mdns/dnssd - install avahi-client/avahi-core!")
                endif()
            endif()
        endif()
    endif()

    if (prefer_dnssd_embedded_api AND LIBAVAHICORE_FOUND)
        set(_PAL_DEP_LIBS ${_PAL_DEP_LIBS} ${LIBAVAHICORE_LIBRARIES})
        set(_PAL_DEP_INC ${_PAL_DEP_INC} ${LIBAVAHICORE_INCLUDEDIR})
        set(_PAL_DEP_DEFS ${_PAL_DEP_DEFS} _AVAHI_EMBEDDED=1)
        set(_PAL_COMPONENTS ${_PAL_COMPONENTS} pal_sd_avahi)
        message(STATUS "  linking with avahi-core")
    elseif (use_dnssd AND LIBAVAHI_FOUND)
        set(_PAL_DEP_LIBS ${_PAL_DEP_LIBS} ${LIBAVAHI_LIBRARIES})
        set(_PAL_DEP_INC ${_PAL_DEP_INC} ${LIBAVAHI_INCLUDEDIR})
        set(_PAL_COMPONENTS ${_PAL_COMPONENTS} pal_sd_avahi)
        message(STATUS "  linking with avahi-client")
    else()
        set(_PAL_COMPONENTS ${_PAL_COMPONENTS} pal_sd_stub)
    endif()

    if(LINUX)
        set(_PAL_COMPONENTS ${_PAL_COMPONENTS}
            pal_ev_epoll
            pal_net_linux
            pal_proc_linux
        )
    else()
        set(use_openssl 1)
        set(_PAL_COMPONENTS ${_PAL_COMPONENTS}
            pal_ev_poll
            pal_net_stubs
            pal_proc_posix
        )
    endif()
    if (NOT use_lws)
        set(_PAL_COMPONENTS ${_PAL_COMPONENTS} pal_ws_stub)
    endif()
endif()
    if (use_lws)
        set(_PAL_COMPONENTS ${_PAL_COMPONENTS} pal_ws_lws)
        message(STATUS "  linking with lws")
    endif()

#
#  Build libwebsockets
#
if (use_lws)
    set(use_openssl 1)
    if(WIN32)
        string(REPLACE "/W4" "/W0" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
        string(REPLACE "/W4" "/W0" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

        set(CMAKE_EXE_LINKER_FLAGS_PREV ${CMAKE_EXE_LINKER_FLAGS})
        set(CMAKE_SHARED_LINKER_FLAGS_PREV ${CMAKE_SHARED_LINKER_FLAGS})
        set(CMAKE_MODULE_LINKER_FLAGS_PREV ${CMAKE_MODULE_LINKER_FLAGS})
        set(CMAKE_STATIC_LINKER_FLAGS_PREV ${CMAKE_MODULE_STATIC_FLAGS})

        set(CMAKE_EXE_LINKER_FLAGS
               "${CMAKE_EXE_LINKER_FLAGS} /IGNORE:4006,4221,4018,4005,4047"
        )
        set(CMAKE_SHARED_LINKER_FLAGS
            "${CMAKE_SHARED_LINKER_FLAGS} /IGNORE:4006,4221,4018,4005,4047"
        )
        set(CMAKE_MODULE_LINKER_FLAGS
            "${CMAKE_MODULE_LINKER_FLAGS} /IGNORE:4006,4221,4018,4005,4047"
        )
        set(CMAKE_STATIC_LINKER_FLAGS
            "${CMAKE_STATIC_LINKER_FLAGS} /IGNORE:4006,4221,4018,4005,4047"
        )
        remove_definitions(/WX)
    endif()
        add_subdirectory(${_LIBWEBSOCKETS_ROOT} ${CMAKE_BINARY_DIR}/libwebsockets)
    if(WIN32)
        add_definitions(/WX)

        set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS_PREV})
        set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS_PREV})
        set(CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS_PREV})
        set(CMAKE_STATIC_LINKER_FLAGS ${CMAKE_STATIC_LINKER_FLAGS_PREV})

        string(REPLACE "/W0" "/W4" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
        string(REPLACE "/W0" "/W4" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    endif()
endif()

if(use_openssl)
    include(FindOpenSSL)
    message(STATUS "  linking with openssl")
endif()

compileAsC99()

#
# proxy shared library target
#
    set(source_c_files
        ${_CMP_ROOT}/cmp.c
        ${_PARSON_ROOT}/parson.c
        ${_HASHTABLE_ROOT}/hashtable.c
        ${_HASHTABLE_ROOT}/hashtable_itr.c
    )
if(WIN32)
    set(source_c_files ${source_c_files} ${_GETOPT_ROOT}/src/getopt.c)
endif()
foreach(unit ${_PROXY_COMPONENTS})
    set(source_c_files ${source_c_files} ${_PROJECT_ROOT}/src/${unit}.c)
endforeach()
foreach(unit ${_PAL_COMPONENTS})
    set(source_c_files ${source_c_files} ${_PROJECT_ROOT}/src/pal/${unit}.c)
endforeach()

    add_library(libproxy SHARED
        $<TARGET_OBJECTS:libiotsdk>
        ${source_c_files}
    )

    set_target_properties(libproxy PROPERTIES PREFIX "")

    target_include_directories(libproxy
        PUBLIC ${_PROJECT_INC}
        PUBLIC ${_IOTSDK_INC}
        PRIVATE ${_UMQTT_INC}
        PRIVATE ${_CMP_ROOT}
        PRIVATE ${_HASHTABLE_ROOT}
        PRIVATE ${_PARSON_ROOT}
    )
if(WIN32)
    target_include_directories(libproxy PRIVATE ${_GETOPT_INC})
endif()
if(${use_lws})
    target_include_directories(libproxy
        PRIVATE ${_LIBWEBSOCKETS_ROOT}/lib
        PRIVATE ${CMAKE_BINARY_DIR}/libwebsockets
    )
endif()
if(use_openssl)
    target_include_directories(libproxy PRIVATE ${OPENSSL_INCLUDE_DIR})
endif()
    target_include_directories(libproxy PRIVATE ${_PAL_DEP_INC})

    target_compile_definitions(libproxy
        PRIVATE _POSIX_C_SOURCE=200112L
        PRIVATE API_EXPORT=1
        PUBLIC HAVE_IN6_U=1
    )

if(${use_zlog})
    target_link_libraries(libproxy PRIVATE libzlog)
endif()
if(WIN32)
    target_link_libraries(libproxy
        PRIVATE winhttp
        PRIVATE ws2_32
        PRIVATE iphlpapi
        PRIVATE secur32
        PRIVATE crypt32
        PRIVATE schannel
    )
else()
    target_link_libraries(libproxy
        PRIVATE ${CURL_LIBRARIES}
        PRIVATE pthread m
    )
endif()
    target_link_libraries(libproxy PRIVATE ${_PAL_DEP_LIBS})
if(use_lws)
    target_link_libraries(libproxy PRIVATE websockets_shared)
endif()
if(use_openssl)
    target_compile_definitions(libproxy PRIVATE USE_OPENSSL=1)
    target_link_libraries(libproxy PRIVATE ${OPENSSL_LIBRARIES})
endif()
    target_compile_definitions(libproxy PRIVATE ${_PAL_DEP_DEFS})

#
# proxy static library target
#
    linkStaticCrt(0)
    add_library(libproxy-static STATIC
        $<TARGET_OBJECTS:libiotsdk>
        ${source_c_files}
    )
    set_target_properties(libproxy-static PROPERTIES PREFIX "")
    set_target_properties(libproxy-static PROPERTIES FOLDER "lib")

    target_include_directories(libproxy-static
        PUBLIC ${_PROJECT_INC}
        PUBLIC ${_IOTSDK_INC}
        PUBLIC ${_UMQTT_INC}
        PUBLIC ${_CMP_ROOT}
        PUBLIC ${_HASHTABLE_ROOT}
        PUBLIC ${_PARSON_ROOT}
    )
if(WIN32)
    target_include_directories(libproxy-static PRIVATE ${_GETOPT_INC})
endif()
    target_compile_definitions(libproxy-static PRIVATE NO_ZLOG=1)
if (use_lws)
    target_include_directories(libproxy-static
        PRIVATE ${_LIBWEBSOCKETS_ROOT}/lib
        PRIVATE ${CMAKE_BINARY_DIR}/libwebsockets
    )
endif()
if(use_openssl)
    target_include_directories(libproxy-static PRIVATE ${OPENSSL_INCLUDE_DIR})
endif()
    target_include_directories(libproxy-static PRIVATE ${_PAL_DEP_INC})

    target_compile_definitions(libproxy-static
        PRIVATE _POSIX_C_SOURCE=200112L
        PRIVATE API_EXPORT=1
        PUBLIC HAVE_IN6_U=1
    )
    target_compile_definitions(libproxy-static PRIVATE ${_PAL_DEP_DEFS})

if(WIN32)
    target_link_libraries(libproxy-static
        PUBLIC winhttp
        PUBLIC ws2_32
        PUBLIC iphlpapi
        PUBLIC secur32
        PUBLIC crypt32
        PUBLIC schannel
    )
else()
    target_link_libraries(libproxy-static
        PRIVATE ${CURL_LIBRARIES}
        PRIVATE pthread m
    )
endif()
if(use_lws)
    target_link_libraries(libproxy-static PUBLIC websockets_shared)
endif()
if(use_openssl)
    target_compile_definitions(libproxy-static PUBLIC USE_OPENSSL=1)
    target_link_libraries(libproxy-static PUBLIC ${OPENSSL_LIBRARIES})
endif()
    target_link_libraries(libproxy-static PUBLIC ${_PAL_DEP_LIBS})

if(WIN32)
  # Place here the install rule for win32
elseif(LINUX)
    install (TARGETS libproxy LIBRARY DESTINATION lib )
endif()
