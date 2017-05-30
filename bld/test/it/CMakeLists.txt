# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

compileAsC99()

if (NOT CMAKE_CROSSCOMPILING)
    add_executable(libproxy-it
        ${_PROJECT_ROOT}/test/it/sd.c
        ${_PROJECT_ROOT}/test/it/ns.c
        ${_PROJECT_ROOT}/test/it/main.c
    )
    set_target_properties(libproxy-it PROPERTIES FOLDER it)
    target_link_libraries(libproxy-it PRIVATE libproxy-static)
    target_compile_definitions(libproxy-it PUBLIC PRIVATE NO_ZLOG=1)
endif()


