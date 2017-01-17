#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#
# Add lib folder
#
function(add_lib_subdirectory folder)
    add_subdirectory(${folder})
    set_target_properties(${folder} PROPERTIES FOLDER "lib")
endfunction()

if(${use_zlog})
add_lib_subdirectory(libzlog)
endif()

add_lib_subdirectory(libiotsdk)
add_lib_subdirectory(libproxy)
add_lib_subdirectory(libuamqp)
add_lib_subdirectory(libumqtt)

add_subdirectory(proxyd)

