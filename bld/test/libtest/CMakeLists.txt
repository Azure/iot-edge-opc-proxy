# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

#
# Build libtest - we build with custom cmake file since we do
# not want to pull in submodules for umock, etc.
#

#
# Source
#
    set(source_c_files
        ${_TRUNNER_ROOT}/src/testmutex.c
        ${_CTEST_ROOT}/src/ctest.c
        ${_UMOCKC_ROOT}/src/umockalloc.c
        ${_UMOCKC_ROOT}/src/umockautoignoreargs.c
        ${_UMOCKC_ROOT}/src/umockcall.c
        ${_UMOCKC_ROOT}/src/umockcallpairs.c
        ${_UMOCKC_ROOT}/src/umockcallrecorder.c
        ${_UMOCKC_ROOT}/src/umockstring.c
        ${_UMOCKC_ROOT}/src/umocktypename.c
        ${_UMOCKC_ROOT}/src/umocktypes.c
        ${_UMOCKC_ROOT}/src/umocktypes_bool.c
        ${_UMOCKC_ROOT}/src/umocktypes_c.c
        ${_UMOCKC_ROOT}/src/umocktypes_charptr.c
        ${_UMOCKC_ROOT}/src/umocktypes_stdint.c
        ${_UMOCKC_ROOT}/src/umock_c.c
        ${_UMOCKC_ROOT}/src/umock_c_negative_tests.c
        ${_UMOCKC_ROOT}/src/umock_log.c
    )
if(WIN32)
    set(source_c_files ${source_c_files}
        ${_PROJECT_TEST}/ut/main.c 
    )
endif()

compileAsC99()

#
# Object Library target
#
    add_library(libtest STATIC ${source_c_files})
    set_property(TARGET libtest PROPERTY POSITION_INDEPENDENT_CODE ON)

    target_include_directories(libtest 
        PRIVATE ${_PROJECT_INC} 
        PUBLIC ${_UMOCKC_INC} 
        PUBLIC ${_TRUNNER_INC} 
        PUBLIC ${_CTEST_INC}
    )
if(WIN32)
else()
    set_target_properties(libtest PROPERTIES PREFIX "")
endif()
