# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

function(build_fuzztest cur_test )
    if (NOT CMAKE_CROSSCOMPILING)
        add_executable(${cur_test}fuzz 
            ${_PROJECT_ROOT}/test/fuzz/${cur_test}.c
            ${_PROJECT_ROOT}/test/fuzz/main.c
        )
        set_target_properties(${cur_test}fuzz PROPERTIES FOLDER fuzz)
        target_link_libraries(${cur_test}fuzz PRIVATE libproxy-static)
        target_compile_definitions(${cur_test}fuzz PUBLIC PRIVATE NO_ZLOG=1)
        if(WIN32)
        else()
            target_link_libraries(${cur_test}fuzz PRIVATE pthread m)
        endif()
    endif()
endfunction()

compileAsC99()

build_fuzztest(proto)
build_fuzztest(cs)
build_fuzztest(ns)
