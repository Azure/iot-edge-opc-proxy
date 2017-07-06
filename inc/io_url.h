// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef _io_url_h_
#define _io_url_h_

#include "common.h"
#include "azure_c_shared_utility/strings.h"
#include "io_token.h"

//
// A url describes a broker endpoint
//
typedef struct io_url
{
    STRING_HANDLE scheme;
    STRING_HANDLE host_name;
    uint16_t port;
    STRING_HANDLE path;
    STRING_HANDLE user_name;
    STRING_HANDLE password;                  // password -- or
    io_token_provider_t* token_provider;             // Tokens
}
io_url_t;

//
// Creates a url from a string
//
decl_internal_2(int32_t, io_url_parse,
    const char*, string,
    io_url_t**, url
);

//
// Creates a url 
//
decl_internal_7(int32_t, io_url_create,
    const char*, scheme,
    const char*, host_name,
    uint16_t, port,
    const char*, path,
    const char*, user_name,
    const char*, password,
    io_url_t**, url
);

//
// Make a clone of the url struct
//
decl_internal_2(int32_t, io_url_clone,
    io_url_t*, url,
    io_url_t**, cloned
);

//
// Returns true if both urls are logically equal
//
decl_internal_2(bool, io_url_equals,
    io_url_t*, that,
    io_url_t*, other
);

//
// Free an address
//
decl_internal_1(void, io_url_free,
    io_url_t*, url
);


#endif // _io_url_h_
