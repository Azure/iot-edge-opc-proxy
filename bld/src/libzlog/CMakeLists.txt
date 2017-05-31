# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

#
# Zlog Source
#
set(source_c_files
        ${_PROJECT_ROOT}/src/util_zlog.c
        ${_ZLOG_ROOT}/src/buf.c
        ${_ZLOG_ROOT}/src/category.c
        ${_ZLOG_ROOT}/src/category_table.c
        ${_ZLOG_ROOT}/src/conf.c
        ${_ZLOG_ROOT}/src/event.c
        ${_ZLOG_ROOT}/src/format.c
        ${_ZLOG_ROOT}/src/level.c
        ${_ZLOG_ROOT}/src/level_list.c
        ${_ZLOG_ROOT}/src/mdc.c
        ${_ZLOG_ROOT}/src/record.c
        ${_ZLOG_ROOT}/src/record_table.c
        ${_ZLOG_ROOT}/src/rotater.c
        ${_ZLOG_ROOT}/src/rule.c
        ${_ZLOG_ROOT}/src/spec.c
        ${_ZLOG_ROOT}/src/thread.c
        ${_ZLOG_ROOT}/src/zc_arraylist.c
        ${_ZLOG_ROOT}/src/zc_hashtable.c
        ${_ZLOG_ROOT}/src/zc_profile.c
        ${_ZLOG_ROOT}/src/zc_threads.c
        ${_ZLOG_ROOT}/src/zc_util.c
        ${_ZLOG_ROOT}/src/zlog.c
)
if(WIN32)
    set(source_c_files ${source_c_files}
        ${_ZLOG_ROOT}/src/zlog_win.c
    )
endif()

compileAsC99()

#
# Zlog based logger shared Library target
#
add_library(libzlog SHARED ${source_c_files})

target_include_directories(libzlog 
    PUBLIC ${_PROJECT_INC}
    PUBLIC ${_ZLOG_INC}
    PRIVATE ${_IOTSDK_INC}
    )

target_compile_definitions(libzlog
    PRIVATE API_EXPORT=1
    )

if(WIN32)
else()
    set_target_properties(libzlog PROPERTIES PREFIX "")
    if(${CMAKE_USE_PTHREADS_INIT})
        target_link_libraries(libzlog PRIVATE pthread)
        target_compile_definitions(libzlog PRIVATE _HAVE_PTHREAD_H=1)
    else()
        message(FATAL_ERROR "No pthread found, please install")
    endif()
endif()

if(WIN32)
  # Place here the install rule for win32
elseif(LINUX)
    install (TARGETS libzlog LIBRARY DESTINATION lib )
endif()
