# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

#
# Build dnssd - we build with custom cmake file so we can include
# what we need.
#

compileAsC99()

#
# Source
#
    set(source_c_files
        ${_DNSSD_ROOT}/src/DebugServices.c
        ${_DNSSD_ROOT}/src/dnssd_clientlib.c
        ${_DNSSD_ROOT}/src/dnssd_clientstub.c
        ${_DNSSD_ROOT}/src/dnssd_ipc.c
        ${_DNSSD_ROOT}/src/GenLinkedList.c
     )
if(WIN32)
    set(source_c_files ${source_c_files}
        ${_DNSSD_ROOT}/src/dllmain.c
        ${_DNSSD_ROOT}/src/dnssd.def
    )
else()
    # todo
endif()

#
# Shared library target
#
    add_library(dnssd SHARED ${source_c_files})
    set_property(TARGET dnssd PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_include_directories(dnssd PUBLIC ${_DNSSD_ROOT}/src)
    
if(WIN32)
    target_compile_definitions(dnssd
        PRIVATE _WINDOWS=1
        PRIVATE _WINSOCK_DEPRECATED_NO_WARNINGS=1
        PRIVATE MDNS_DEBUGMSGS=0
        PRIVATE WIN32_LEAN_AND_MEAN=1
        PRIVATE NOT_HAVE_SA_LEN=1
        PRIVATE USE_TCP_LOOPBACK=1
        PRIVATE _CRT_SECURE_NO_DEPRECATE=1
        PRIVATE _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1
        PRIVATE mdns_EXPORTS=1
    )
    target_link_libraries(dnssd
        PRIVATE ws2_32 
        PRIVATE iphlpapi
        PRIVATE secur32 
        PRIVATE crypt32 
        PRIVATE schannel
    )
else()
    # todo
endif()

