# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

compileAsC99()

#
# Proxyd executable target
#
add_executable(proxyd ${_PROJECT_ROOT}/src/prx_main.c)

target_link_libraries(proxyd libproxy)
if(${use_zlog})
    target_link_libraries(proxyd libzlog)
endif()

if(CMAKE_CROSSCOMPILING)
  if(LINUX)
    # If cross compiling with we need add the shared libraries used by the libraries above.
    # As mentioned in the bottom of
    # https://cmake.org/Wiki/CMake_Cross_Compiling
    # the pkg-config tool can not be used by cmake while cross compiling.
    # So we need cmake explicitly to link against the libaries which are
    # needed by the already used libraries.
    #
    # Libraries needed by libproxy:
    if(${use_openssl})
      target_link_libraries(proxyd ${OPENSSL_LIBRARIES})
    endif()
    target_link_libraries(proxyd ${ZLIB_LIBRARY})
    # Libraries needed by curl (which is introduced to the static linked libwebsocket):
    target_link_libraries(proxyd ${GSSAPI_KRB5_LIBRARY})
    target_link_libraries(proxyd ${KRB5_LIBRARY})
    target_link_libraries(proxyd ${COM_ERR_LIBRARY})
    target_link_libraries(proxyd ${IDN_LIBRARY})
    target_link_libraries(proxyd ${K5CRYPTO_LIBRARY})
    target_link_libraries(proxyd ${KEYUTILS_LIBRARY})
    target_link_libraries(proxyd ${KRB5SUPPORT_LIBRARY})
    target_link_libraries(proxyd ${LBER-2.4_LIBRARY})
    target_link_libraries(proxyd ${LDAP_R-2.4_LIBRARY})
    target_link_libraries(proxyd ${RTMP_LIBRARY})
    target_link_libraries(proxyd ${SSH2_LIBRARY})
    target_link_libraries(proxyd ${GCRYPT_LIBRARY})
    target_link_libraries(proxyd ${SASL2_LIBRARY})
    target_link_libraries(proxyd ${GNUTLS_DEB0_LIBRARY})
    target_link_libraries(proxyd ${HOGWEED_LIBRARY})
    target_link_libraries(proxyd ${NETTLE_LIBRARY})
    target_link_libraries(proxyd ${GMP_LIBRARY})
    target_link_libraries(proxyd ${GPG_ERROR_LIBRARY})
    target_link_libraries(proxyd ${P11_KIT_LIBRARY})
    target_link_libraries(proxyd ${TASN1_LIBRARY})
    target_link_libraries(proxyd ${FFI_LIBRARY})
    target_link_libraries(proxyd ${DBUS_LIBRARY})
  endif()
endif()

if(WIN32)
    # Place here the install rule for win32
elseif(LINUX)
    install (TARGETS proxyd RUNTIME DESTINATION bin )
endif()
