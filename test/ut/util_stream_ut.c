// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST util_stream
#include "util_ut.h"

//
// 1. Required mocks
//
#include "azure_c_shared_utility/strings.h"
#include "prx_buffer.h"

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include UNIT_H
#define ENABLE_MOCKS
#include UNIT_C

//
// 3. Setup test suite
//
BEGIN_DECLARE_TEST_SUITE()
END_DECLARE_TEST_SUITE()
// -or- DECLARE_TEST_SUITE()

//
// 4. Setup and run tests
//
DECLARE_TEST_SETUP()

#ifdef io_stream_readable //
// Test io_stream_readable happy path
//
TEST_FUNCTION(io_stream_readable__success)
{
    static const io_stream_t* k_stream_valid;
    size_t result;

    // arrange
    // ...

    // act
    result = io_stream_readable(k_stream_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, er_ok, result);
    // ...
}

//
// Test io_stream_readable passing as stream argument an invalid io_stream_t* value
//
TEST_FUNCTION(io_stream_readable__arg_stream_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_stream_readable();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_stream_readable unhappy path
//
TEST_FUNCTION(io_stream_readable__neg)
{
    static const io_stream_t* k_stream_valid;
    size_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_stream_readable(k_stream_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(size_t, result, er_ok);
}

#endif // io_stream_readable

#ifdef io_stream_reset //
// Test io_stream_reset happy path
//
TEST_FUNCTION(io_stream_reset__success)
{
    static const io_stream_t* k_stream_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_stream_reset(k_stream_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_stream_reset passing as stream argument an invalid io_stream_t* value
//
TEST_FUNCTION(io_stream_reset__arg_stream_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_stream_reset();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_stream_reset unhappy path
//
TEST_FUNCTION(io_stream_reset__neg)
{
    static const io_stream_t* k_stream_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_stream_reset(k_stream_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_stream_reset

#ifdef io_stream_read //
// Test io_stream_read happy path
//
TEST_FUNCTION(io_stream_read__success)
{
    static const io_stream_t* k_stream_valid;
    static const void* k_buf_valid;
    static const size_t k_len_valid;
    static const size_t* k_read_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_stream_read(k_stream_valid, k_buf_valid, k_len_valid, k_read_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_stream_read passing as stream argument an invalid io_stream_t* value
//
TEST_FUNCTION(io_stream_read__arg_stream_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_stream_read();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_stream_read passing as buf argument an invalid void* value
//
TEST_FUNCTION(io_stream_read__arg_buf_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_stream_read();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_stream_read passing as len argument an invalid size_t value
//
TEST_FUNCTION(io_stream_read__arg_len_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_stream_read();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_stream_read passing as read argument an invalid size_t* value
//
TEST_FUNCTION(io_stream_read__arg_read_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_stream_read();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_stream_read unhappy path
//
TEST_FUNCTION(io_stream_read__neg)
{
    static const io_stream_t* k_stream_valid;
    static const void* k_buf_valid;
    static const size_t k_len_valid;
    static const size_t* k_read_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_stream_read(k_stream_valid, k_buf_valid, k_len_valid, k_read_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_stream_read

#ifdef io_stream_writeable //
// Test io_stream_writeable happy path
//
TEST_FUNCTION(io_stream_writeable__success)
{
    static const io_stream_t* k_stream_valid;
    size_t result;

    // arrange
    // ...

    // act
    result = io_stream_writeable(k_stream_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(size_t, er_ok, result);
    // ...
}

//
// Test io_stream_writeable passing as stream argument an invalid io_stream_t* value
//
TEST_FUNCTION(io_stream_writeable__arg_stream_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_stream_writeable();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_stream_writeable unhappy path
//
TEST_FUNCTION(io_stream_writeable__neg)
{
    static const io_stream_t* k_stream_valid;
    size_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_stream_writeable(k_stream_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(size_t, result, er_ok);
}

#endif // io_stream_writeable

#ifdef io_stream_write //
// Test io_stream_write happy path
//
TEST_FUNCTION(io_stream_write__success)
{
    static const io_stream_t* k_stream_valid;
    static const const void* k_buf_valid;
    static const size_t k_len_valid;
    int32_t result;

    // arrange
    // ...

    // act
    result = io_stream_write(k_stream_valid, k_buf_valid, k_len_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_ok, result);
    // ...
}

//
// Test io_stream_write passing as stream argument an invalid io_stream_t* value
//
TEST_FUNCTION(io_stream_write__arg_stream_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_stream_write();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_stream_write passing as buf argument an invalid const void* value
//
TEST_FUNCTION(io_stream_write__arg_buf_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_stream_write();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_stream_write passing as len argument an invalid size_t value
//
TEST_FUNCTION(io_stream_write__arg_len_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_stream_write();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_stream_write unhappy path
//
TEST_FUNCTION(io_stream_write__neg)
{
    static const io_stream_t* k_stream_valid;
    static const const void* k_buf_valid;
    static const size_t k_len_valid;
    int32_t result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_stream_write(k_stream_valid, k_buf_valid, k_len_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(int32_t, result, er_ok);
}

#endif // io_stream_write

#ifdef io_stream_close //
// Test io_stream_close happy path
//
TEST_FUNCTION(io_stream_close__success)
{
    static const io_stream_t* k_stream_valid;
    void result;

    // arrange
    // ...

    // act
    result = io_stream_close(k_stream_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(void, er_ok, result);
    // ...
}

//
// Test io_stream_close passing as stream argument an invalid io_stream_t* value
//
TEST_FUNCTION(io_stream_close__arg_stream_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_stream_close();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_stream_close unhappy path
//
TEST_FUNCTION(io_stream_close__neg)
{
    static const io_stream_t* k_stream_valid;
    void result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_stream_close(k_stream_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(void, result, er_ok);
}

#endif // io_stream_close


#ifdef io_fixed_buffer_stream_init

//
//Test io_fixed_buffer_stream_init happy path
//
TEST_FUNCTION(io_fixed_buffer_stream_init__success)
{
    static const io_fixed_buffer_stream_t* k_mem_valid;
    static const const uint8_t* k_in_valid;
    static const size_t k_in_len_valid;
    static const uint8_t* k_out_valid;
    static const size_t k_out_len_valid;
    io_stream_t* result;

    // arrange
    // ...

    // act
    result = io_fixed_buffer_stream_init(k_mem_valid, k_in_valid, k_in_len_valid, k_out_valid, k_out_len_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(io_stream_t*, er_ok, result);
    // ...
}

//
// Test io_fixed_buffer_stream_init passing as mem argument an invalid io_fixed_buffer_stream_t* value
//
TEST_FUNCTION(io_fixed_buffer_stream_init__arg_mem_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_fixed_buffer_stream_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_fixed_buffer_stream_init passing as in argument an invalid const uint8_t* value
//
TEST_FUNCTION(io_fixed_buffer_stream_init__arg_in_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_fixed_buffer_stream_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_fixed_buffer_stream_init passing as in_len argument an invalid size_t value
//
TEST_FUNCTION(io_fixed_buffer_stream_init__arg_in_len_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_fixed_buffer_stream_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_fixed_buffer_stream_init passing as out argument an invalid uint8_t* value
//
TEST_FUNCTION(io_fixed_buffer_stream_init__arg_out_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_fixed_buffer_stream_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_fixed_buffer_stream_init passing as out_len argument an invalid size_t value
//
TEST_FUNCTION(io_fixed_buffer_stream_init__arg_out_len_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_fixed_buffer_stream_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_fixed_buffer_stream_init unhappy path
//
TEST_FUNCTION(io_fixed_buffer_stream_init__neg)
{
    static const io_fixed_buffer_stream_t* k_mem_valid;
    static const const uint8_t* k_in_valid;
    static const size_t k_in_len_valid;
    static const uint8_t* k_out_valid;
    static const size_t k_out_len_valid;
    io_stream_t* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_fixed_buffer_stream_init(k_mem_valid, k_in_valid, k_in_len_valid, k_out_valid, k_out_len_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(io_stream_t*, result, er_ok);
}

#endif // io_fixed_buffer_stream_init;

#ifdef io_dynamic_buffer_stream_init

//
//Test io_dynamic_buffer_stream_init happy path
//
TEST_FUNCTION(io_dynamic_buffer_stream_init__success)
{
    static const io_dynamic_buffer_stream_t* k_mem_valid;
    static const prx_buffer_factory_t* k_pool_valid;
    static const size_t k_increment_valid;
    io_stream_t* result;

    // arrange
    // ...

    // act
    result = io_dynamic_buffer_stream_init(k_mem_valid, k_pool_valid, k_increment_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(io_stream_t*, er_ok, result);
    // ...
}

//
// Test io_dynamic_buffer_stream_init passing as mem argument an invalid io_dynamic_buffer_stream_t* value
//
TEST_FUNCTION(io_dynamic_buffer_stream_init__arg_mem_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_dynamic_buffer_stream_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_dynamic_buffer_stream_init passing as pool argument an invalid prx_buffer_factory_t* value
//
TEST_FUNCTION(io_dynamic_buffer_stream_init__arg_pool_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_dynamic_buffer_stream_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_dynamic_buffer_stream_init passing as increment argument an invalid size_t value
//
TEST_FUNCTION(io_dynamic_buffer_stream_init__arg_increment_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_dynamic_buffer_stream_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_dynamic_buffer_stream_init unhappy path
//
TEST_FUNCTION(io_dynamic_buffer_stream_init__neg)
{
    static const io_dynamic_buffer_stream_t* k_mem_valid;
    static const prx_buffer_factory_t* k_pool_valid;
    static const size_t k_increment_valid;
    io_stream_t* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_dynamic_buffer_stream_init(k_mem_valid, k_pool_valid, k_increment_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(io_stream_t*, result, er_ok);
}

#endif // io_dynamic_buffer_stream_init;

#ifdef io_file_stream_init

//
//Test io_file_stream_init happy path
//
TEST_FUNCTION(io_file_stream_init__success)
{
    static const io_file_stream_t* k_fs_valid;
    static const const char* k_file_name_valid;
    static const const char* k_mode_valid;
    io_stream_t* result;

    // arrange
    // ...

    // act
    result = io_file_stream_init(k_fs_valid, k_file_name_valid, k_mode_valid);

    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(io_stream_t*, er_ok, result);
    // ...
}

//
// Test io_file_stream_init passing as fs argument an invalid io_file_stream_t* value
//
TEST_FUNCTION(io_file_stream_init__arg_fs_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_file_stream_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_file_stream_init passing as file_name argument an invalid const char* value
//
TEST_FUNCTION(io_file_stream_init__arg_file_name_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_file_stream_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_file_stream_init passing as mode argument an invalid const char* value
//
TEST_FUNCTION(io_file_stream_init__arg_mode_invalid)
{
    // ...
    int32_t result;

    // arrange
    // ...

    // act
    handle = io_file_stream_init();

    // assert
    ASSERT_EXPECTED_CALLS();
    ASSERT_ARE_EQUAL(int32_t, er_fault, result);
    // ...
}

//
// Test io_file_stream_init unhappy path
//
TEST_FUNCTION(io_file_stream_init__neg)
{
    static const io_file_stream_t* k_fs_valid;
    static const const char* k_file_name_valid;
    static const const char* k_mode_valid;
    io_stream_t* result;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = io_file_stream_init(k_fs_valid, k_file_name_valid, k_mode_valid);

    // assert
    UMOCK_C_NEGATIVE_TESTS_ASSERT(io_stream_t*, result, er_ok);
}

#endif // io_file_stream_init;



//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

