// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST prx_host
#include "util_ut.h"

//
// 1. Required mocks
//
#include "getopt.h"
#include "util_misc.h"
#include "io_ref.h"
#include "io_cs.h"
#include "prx_ns.h"
#include "prx_sched.h"

char *optarg;   /* pointer to argument of current option  */

MOCKABLE_FUNCTION(, int, getopt_long, int, nargc, char* const*, nargv, const char*, options,
    const struct option*, long_options, int*, idx);

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "io_host.h"
#include "azure_c_shared_utility/tickcounter.h"
#include UNIT_H
#define ENABLE_MOCKS
#include UNIT_C

// io_transport.h
MOCKABLE_FUNCTION(, io_transport_t*, io_iot_hub_mqtt_server_transport);

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
END_DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()


#ifdef prx_host_init
//
// Test prx_host_init happy path
//
TEST_FUNCTION(prx_host_init__success)
{
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_host_init();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_host_init unhappy path
//
TEST_FUNCTION(prx_host_init__neg)
{
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_host_init();

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_host_init;

#ifdef prx_host_console //
// Test prx_host_console happy path
//
TEST_FUNCTION(prx_host_console__success)
{
    static const const char* k_config_file_valid;
    static const int32_t k_argc_valid;
    static const char** k_argv_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_host_console(k_config_file_valid, k_argc_valid, k_argv_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_host_console passing as config_file argument an invalid const char* value
//
TEST_FUNCTION(prx_host_console__arg_config_file_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_host_console();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_host_console passing as argc argument an invalid int32_t value
//
TEST_FUNCTION(prx_host_console__arg_argc_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_host_console();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_host_console passing as argv argument an invalid char** value
//
TEST_FUNCTION(prx_host_console__arg_argv_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_host_console();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_host_console unhappy path
//
TEST_FUNCTION(prx_host_console__neg)
{
    static const const char* k_config_file_valid;
    static const int32_t k_argc_valid;
    static const char** k_argv_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_host_console(k_config_file_valid, k_argc_valid, k_argv_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_host_console;

#ifdef prx_host_create //
// Test prx_host_create happy path
//
TEST_FUNCTION(prx_host_create__success)
{
    static const const char* k_config_file_valid;
    static const prx_host_type_t k_type_valid;
    static const prx_host_t** k_prx_host_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_host_create(k_config_file_valid, k_type_valid, k_prx_host_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_host_create passing as config_file argument an invalid const char* value
//
TEST_FUNCTION(prx_host_create__arg_config_file_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_host_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_host_create passing as type argument an invalid prx_host_type_t value
//
TEST_FUNCTION(prx_host_create__arg_type_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_host_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_host_create passing as host argument an invalid prx_host_t** value
//
TEST_FUNCTION(prx_host_create__arg_prx_host_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_host_create();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_host_create unhappy path
//
TEST_FUNCTION(prx_host_create__neg)
{
    static const const char* k_config_file_valid;
    static const prx_host_type_t k_type_valid;
    static const prx_host_t** k_prx_host_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_host_create(k_config_file_valid, k_type_valid, k_prx_host_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_host_create;

#ifdef prx_host_start //
// Test prx_host_start happy path
//
TEST_FUNCTION(prx_host_start__success)
{
    static const prx_host_t* k_prx_host_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_host_start(k_prx_host_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_host_start passing as host argument an invalid prx_host_t* value
//
TEST_FUNCTION(prx_host_start__arg_prx_host_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_host_start();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_host_start unhappy path
//
TEST_FUNCTION(prx_host_start__neg)
{
    static const prx_host_t* k_prx_host_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_host_start(k_prx_host_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_host_start;

#ifdef prx_host_sig_wait //
// Test prx_host_sig_wait happy path
//
TEST_FUNCTION(prx_host_sig_wait__success)
{
    static const prx_host_t* k_prx_host_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_host_sig_wait(k_prx_host_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_host_sig_wait passing as host argument an invalid prx_host_t* value
//
TEST_FUNCTION(prx_host_sig_wait__arg_prx_host_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_host_sig_wait();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_host_sig_wait unhappy path
//
TEST_FUNCTION(prx_host_sig_wait__neg)
{
    static const prx_host_t* k_prx_host_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_host_sig_wait(k_prx_host_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_host_sig_wait;

#ifdef prx_host_sig_break //
// Test prx_host_sig_break happy path
//
TEST_FUNCTION(prx_host_sig_break__success)
{
    static const prx_host_t* k_prx_host_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_host_sig_break(k_prx_host_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_host_sig_break passing as host argument an invalid prx_host_t* value
//
TEST_FUNCTION(prx_host_sig_break__arg_prx_host_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_host_sig_break();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_host_sig_break unhappy path
//
TEST_FUNCTION(prx_host_sig_break__neg)
{
    static const prx_host_t* k_prx_host_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_host_sig_break(k_prx_host_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_host_sig_break;

#ifdef prx_host_stop //
// Test prx_host_stop happy path
//
TEST_FUNCTION(prx_host_stop__success)
{
    static const prx_host_t* k_prx_host_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_host_stop(k_prx_host_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_host_stop passing as host argument an invalid prx_host_t* value
//
TEST_FUNCTION(prx_host_stop__arg_prx_host_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_host_stop();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_host_stop unhappy path
//
TEST_FUNCTION(prx_host_stop__neg)
{
    static const prx_host_t* k_prx_host_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_host_stop(k_prx_host_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_host_stop;

#ifdef prx_host_clone //
// Test prx_host_clone happy path
//
TEST_FUNCTION(prx_host_clone__success)
{
    static const prx_host_t* k_prx_host_valid;
    static const prx_host_t** k_cloned_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = prx_host_clone(k_prx_host_valid, k_cloned_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test prx_host_clone passing as host argument an invalid prx_host_t* value
//
TEST_FUNCTION(prx_host_clone__arg_prx_host_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_host_clone();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_host_clone passing as cloned argument an invalid prx_host_t** value
//
TEST_FUNCTION(prx_host_clone__arg_cloned_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_host_clone();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_host_clone unhappy path
//
TEST_FUNCTION(prx_host_clone__neg)
{
    static const prx_host_t* k_prx_host_valid;
    static const prx_host_t** k_cloned_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_host_clone(k_prx_host_valid, k_cloned_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // prx_host_clone;

#ifdef prx_host_release //
// Test prx_host_release happy path
//
TEST_FUNCTION(prx_host_release__success)
{
    static const prx_host_t* k_prx_host_valid;
    void result;

    // arrange
    // ...

    // act
    result = prx_host_release(k_prx_host_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ...
}

//
// Test prx_host_release passing as host argument an invalid prx_host_t* value
//
TEST_FUNCTION(prx_host_release__arg_prx_host_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = prx_host_release();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test prx_host_release unhappy path
//
TEST_FUNCTION(prx_host_release__neg)
{
    static const prx_host_t* k_prx_host_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_host_release(k_prx_host_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // prx_host_release;

#ifdef prx_host_deinit //
// Test prx_host_deinit happy path
//
TEST_FUNCTION(prx_host_deinit__success)
{
    void result;

    // arrange
    // ...

    // act
    result = prx_host_deinit();

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ...
}

//
// Test prx_host_deinit unhappy path
//
TEST_FUNCTION(prx_host_deinit__neg)
{
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = prx_host_deinit();

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // prx_host_deinit;

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

