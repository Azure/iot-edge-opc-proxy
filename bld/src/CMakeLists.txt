#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#
# Macro that enables or disables linking with
#
macro(linkStaticCrt enabled)
if(MSVC AND NOT ${link_msvcrt_default})
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_C_FLAGS
        CMAKE_CXX_FLAGS_DEBUG CMAKE_C_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE CMAKE_C_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_C_FLAGS_MINSIZEREL
        CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS_RELWITHDEBINFO)
        if (enabled)
            if(${flag_var} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
            endif()
        else()
            if(${flag_var} MATCHES "/MT")
                string(REGEX REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
            endif()
        endif()
    endforeach()
endif()
endmacro()

#
# Add lib folder
#
function(add_lib_subdirectory folder)
    add_subdirectory(${folder})
    set_target_properties(${folder} PROPERTIES FOLDER "lib")
endfunction()

linkStaticCrt(1)
if(${use_zlog})
    add_lib_subdirectory(libzlog)
endif()
if(WIN32)
    add_subdirectory(libdnssd)
    set_target_properties(dnssd PROPERTIES FOLDER "lib")
endif()

add_lib_subdirectory(libiotsdk)
add_lib_subdirectory(libproxy)
linkStaticCrt(1)
add_subdirectory(proxyd)
linkStaticCrt(0)
