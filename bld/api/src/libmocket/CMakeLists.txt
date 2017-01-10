# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

#
# Libmocket shared library target
#
#add_library(libmocket SHARED 
#	${_PROJECT_ROOT}/src/api_mocket.c
#    )
#set_target_properties(libmocket PROPERTIES PREFIX "")
#
#target_include_directories(libmocket 
#    PUBLIC ${_PROJECT_INC}
#    PRIVATE ${_IOTSDK_INC}
#    )
#
#target_compile_definitions(libmocket 
#    PRIVATE API_EXPORT=1
#    PRIVATE LIB_MOCKET=1
#    PUBLIC HAVE_IN6_U=1
#    )
#
#target_link_libraries(libmocket 
#    PRIVATE libproxy
#    )
