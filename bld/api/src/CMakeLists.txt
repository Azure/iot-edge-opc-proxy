#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#
# Add lib folder
#
function(add_lib_subdirectory folder)
    add_subdirectory(${folder})
    set_target_properties(${folder} PROPERTIES FOLDER "lib")
endfunction()

add_lib_subdirectory(libmocket)
