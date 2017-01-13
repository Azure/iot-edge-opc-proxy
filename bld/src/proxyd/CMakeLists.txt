# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

#
# Proxyd executable target
#
add_executable(proxyd ${_PROJECT_ROOT}/src/prx_main.c)

target_link_libraries(proxyd libproxy)
if(${use_zlog})
target_link_libraries(proxyd libzlog)
endif()