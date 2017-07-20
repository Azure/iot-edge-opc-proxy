# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
endif()

#
# Directory variables that are used across all projects
#
set(_GIT_ROOT ${_PROJECT_ROOT}/.. CACHE INTERNAL "" FORCE)
set(_DEPS_ROOT ${_PROJECT_ROOT}/deps CACHE INTERNAL "" FORCE)

#
# This project's folders
#
set(_PROJECT_INC ${_PROJECT_ROOT}/inc CACHE INTERNAL "" FORCE)
set(_PROJECT_SRC ${_PROJECT_ROOT}/src CACHE INTERNAL "" FORCE)
set(_PROJECT_TEST ${_PROJECT_ROOT}/test CACHE INTERNAL "" FORCE)

set(_API_ROOT ${_PROJECT_ROOT}/api/c CACHE INTERNAL "" FORCE)
set(_API_SRC ${_API_ROOT}/src CACHE INTERNAL "" FORCE)
set(_API_SAMPLES ${_API_ROOT}/samples CACHE INTERNAL "" FORCE)

#
# Configurable external dependency paths, from submodules
#
set(_CMP_ROOT ${_DEPS_ROOT}/cmp CACHE PATH "Cmp Root")
set(_GETOPT_ROOT ${_DEPS_ROOT}/getopt CACHE PATH "Getopt Root")
set(_PARSON_ROOT ${_DEPS_ROOT}/parson CACHE PATH "Parson Root")
set(_ZLOG_ROOT ${_DEPS_ROOT}/zlog CACHE PATH "Zlog Root")
set(_HASHTABLE_ROOT ${_DEPS_ROOT}/hashtable CACHE PATH "Hashtable Root")
set(_UMQTT_ROOT ${_DEPS_ROOT}/umqtt CACHE PATH "Azure uMQTT Root")
set(_SHARED_ROOT ${_DEPS_ROOT}/shared CACHE PATH "Azure C Shared Root")
set(_LIBWEBSOCKETS_ROOT ${_DEPS_ROOT}/lws CACHE PATH "Libwebsockets Root")
set(_DNSSD_ROOT ${_DEPS_ROOT}/dnssd CACHE PATH "Dns-SD Root")
set(_UMOCKC_ROOT ${_DEPS_ROOT}/umock CACHE PATH "Azure uMock C Root")
set(_TRUNNER_ROOT ${_DEPS_ROOT}/trunner CACHE PATH "Azure test runner Root")
set(_CTEST_ROOT ${_DEPS_ROOT}/ctest CACHE PATH "Azure ctest harness Root")

#
# Internal include folders of external dependencies
#
set(_UMQTT_INC ${_UMQTT_ROOT}/inc CACHE INTERNAL "" FORCE)
set(_IOTSDK_INC ${_SHARED_ROOT}/inc CACHE INTERNAL "" FORCE)
set(_IOTSDK_SRC ${_SHARED_ROOT}/src CACHE INTERNAL "" FORCE)
set(_ZLOG_INC ${_ZLOG_ROOT}/src CACHE INTERNAL "" FORCE)
set(_UMOCKC_INC ${_UMOCKC_ROOT}/inc CACHE INTERNAL "" FORCE)
set(_TRUNNER_INC ${_TRUNNER_ROOT}/inc CACHE INTERNAL "" FORCE)
set(_CTEST_INC ${_CTEST_ROOT}/inc CACHE INTERNAL "" FORCE)
set(_GETOPT_INC ${_GETOPT_ROOT}/src CACHE INTERNAL "" FORCE)

#
# Compiler and Linker settings
#
    option(compile_options_C "passes a string to the command line of the C compiler" OFF)
if(NOT "${compile_options_C}" STREQUAL "OFF")
    set(CMAKE_C_FLAGS "${compile_options_C} ${CMAKE_C_FLAGS}")
endif()

    option(compile_options_CXX "passes a string to the command line of the C++ compiler" OFF)
if(NOT "${compile_options_CXX}" STREQUAL "OFF")
    set(CMAKE_CXX_FLAGS "${compile_options_CXX} ${CMAKE_CXX_FLAGS}")
endif()

if(WIN32)
    option(use_lws
        "set use_lws to ON to use libwebsockets (default is OFF)" OFF)
    option(use_openssl
        "set use_openssl to ON to use openssl instead of schannel (default is OFF)" OFF)
else()
    option(use_lws
        "set use_lws to OFF to not use libwebsockets (default is ON)" ON)
    option(use_openssl
        "set use_openssl to OFF to use a different library (default is ON)" ON)
endif()

    option(use_dnssd
        "set use_dnssd to OFF to prevent the use of dnssd and mdns (default is ON)" ON)
    option(prefer_dnssd_embedded_api
        "set prefer_dnssd_embedded_api to ON to try and not use the sd daemon (default is OFF)" OFF)

    option(use_zlog
        "set use_zlog to ON to use zlog as logging library (default is OFF)" OFF)
    if(NOT ${use_zlog})
        add_definitions(-DNO_ZLOG)
    endif()

    option(mem_check
        "set mem_check to ON (default is OFF)" OFF)
    if(${mem_check})
        add_definitions(-DMEM_CHECK)
    endif()
    option(leak_detection
        "set leak_detection to ON (default is OFF)" OFF)
    if(${leak_detection})
        add_definitions(-DLEAK_DETECT)
    endif()
if(WIN32)
    if(MSVC)
        option(link_dynamic_msvcrt
            "set link_dynamic_msvcrt to ON to link to msvcrt dll (default is OFF)" OFF)

        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        if(${leak_detection} OR ${mem_check})
            add_definitions(-D_CRTDBG_MAP_ALLOC)
        endif()
    endif()
else()
    #
    # Find external dependencies
    #
    set(CMAKE_THREAD_PREFER_PTHREAD 1)
    include(FindThreads)

    include(FindCURL)
    if(${CURL_FOUND})
    else()
        message(FATAL_ERROR "No CURL found, please install")
    endif()

    include(CheckIncludeFile)
    check_include_file("getopt.h" HAVE_GETOPT_H)
    if(HAVE_GETOPT_H)
    else()
        message(FATAL_ERROR "getopt not found" )
    endif()

    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        add_definitions(-D_DEBUG)
    endif()
    
    #
    # Try to define the version from the closest parent git tag and
    # the current Git hash.
    #
    find_package(Git)
    if(GIT_EXECUTABLE)
        execute_process(
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMAND "${GIT_EXECUTABLE}" describe --always --tags --dirty
            OUTPUT_VARIABLE GIT_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()
endif()

if (NOT GIT_VERSION)
    add_definitions(-DSCM_VERSION="${GIT_VERSION}")
endif()

#
# Macro to compile as c99
#
macro(compileAsC99)
if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
        set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
    endif()
else()
    set (CMAKE_C_STANDARD 99)
    set (CMAKE_CXX_STANDARD 11)
endif()
endmacro(compileAsC99)

#
# Add files to be installed on system
#
function(add_files_to_install files)
    set(INSTALL_H_FILES ${INSTALL_H_FILES} ${files} CACHE INTERNAL "Installed headers")
endfunction()

add_subdirectory(src)
add_subdirectory(api)

option(run_unittests "set run_unittests to ON to run unittests (default is OFF)" OFF)

#
# Build a test
#
function(build_ut cur_test folder)
    if (run_unittests AND NOT CMAKE_CROSSCOMPILING)
        add_executable(${cur_test}_exe ${${cur_test}_test_files}
            ${${cur_test}_c_files}
        )
        set_target_properties(${cur_test}_exe PROPERTIES FOLDER ${folder})
        target_compile_definitions(${cur_test}_exe PUBLIC -DUSE_CTEST)
        target_include_directories(${cur_test}_exe PUBLIC ${${cur_test}_test_includes})

        if(WIN32)
            target_link_libraries(${cur_test}_exe libtest ${ARGN})
            add_test(NAME ${cur_test} COMMAND ${cur_test}_exe)
        else()
            target_link_libraries(${cur_test}_exe libtest m ${ARGN})
            # this part detects
            #    - the additional libraries that might be needed.
            #     additional libraries are started by ADDITIONAL_LIBS parameter and ended by any
            #     other known parameter (or end of variable arguments)
            #   - a valgring suppression file (VALGRIND_SUPRESSIONS_FILE) for memcheck
            #     the file name follows immediately after
            set(PARSING_ADDITIONAL_LIBS OFF)
            set(PARSING_VALGRIND_SUPRESSIONS_FILE OFF)
            set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER)
            foreach(f ${ARGN})
                if(${f} STREQUAL "ADDITIONAL_LIBS")
                    SET(PARSING_ADDITIONAL_LIBS ON)
                    SET(PARSING_VALGRIND_SUPRESSION_FILE OFF)
                    #also unset all the other states
                    continue()
                elseif(${f} STREQUAL "VALGRIND_SUPPRESSIONS_FILE")
                    SET(PARSING_ADDITIONAL_LIBS OFF)
                    SET(PARSING_VALGRIND_SUPPRESSION_FILE ON)
                    continue()
                endif()
                if(PARSING_ADDITIONAL_LIBS)
                    target_link_libraries(${cur_test}_exe ${f})
                elseif(PARSING_VALGRIND_SUPPRESSION_FILE)
                    set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER "--suppressions=${f}")
                endif()
            endforeach()

            add_test(NAME ${cur_test} COMMAND $<TARGET_FILE:${cur_test}_exe>)

            find_program(VALGRIND_FOUND NAMES valgrind)
            if(${VALGRIND_FOUND} STREQUAL VALGRIND_FOUND-NOTFOUND)
            else()
                add_test(
                    NAME ${cur_test}_valgrind COMMAND valgrind
                        --num-callers=50 --error-exitcode=1
                        --trace-children=yes --leak-check=full --track-origins=yes
                        ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER}
                        $<TARGET_FILE:${cur_test}_exe>)
                add_test(
                    NAME ${cur_test}_helgrind COMMAND valgrind --tool=helgrind
                        --num-callers=50 --error-exitcode=1
                        $<TARGET_FILE:${cur_test}_exe>)
            endif()
        endif()
    endif()
endfunction()

add_subdirectory(test)
