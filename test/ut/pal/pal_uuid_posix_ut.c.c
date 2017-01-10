// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST pal_uuid_linux
#include "util_ut.h"

//
// 1. Required mocks
//
#include "os.h"
#include "api_types.h"

//
// 2. Include unit under test
//
#undef ENABLE_MOCKS
#include "pal_uuid.h"
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

#if 0

// 
// Test pal_init happy path 
// 
TEST_FUNCTION(pal_posix_init__success)
{
    static const char* k_config_valid = fill_in_here;

    // arrange 
    // ... 

    // act 
    result = pal_init(k_config_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_init passing as config argument an invalid const char* value 
// 
TEST_FUNCTION(pal_posix_init__arg_config_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = pal_init();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_init unhappy path 
// 
TEST_FUNCTION(pal_posix_init__neg)
{
    static const char* k_config_valid = fill_in_here;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_init(k_config_valid);

    // assert    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line18 "pal_misc.h"




typedef void(*pal_diag_callback_t)(
    const char* target,
    const char* msg
    );




// 
// Test pal_set_diag_callback happy path 
// 
TEST_FUNCTION(pal_posix_set_diag_callback__success)
{
    const pal_diag_callback_t k_cb_valid = fill_in_here;

    // arrange 
    // ... 

    // act 
    result = pal_set_diag_callback(k_cb_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_set_diag_callback passing as cb argument an invalid pal_diag_callback_t value 
// 
TEST_FUNCTION(pal_posix_set_diag_callback__arg_cb_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = pal_set_diag_callback();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_set_diag_callback unhappy path 
// 
TEST_FUNCTION(pal_posix_set_diag_callback__neg)
{
    const pal_diag_callback_t k_cb_valid = fill_in_here;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_set_diag_callback(k_cb_valid);

    // assert    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line33 "pal_misc.h"




// 
// Test pal_init_uuid happy path 
// 
TEST_FUNCTION(pal_posix_init_uuid__success)
{
    const pal_uuid_t k_uuid_valid = fill_in_here;

    // arrange 
    // ... 

    // act 
    result = pal_init_uuid(k_uuid_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_init_uuid passing as uuid argument an invalid pal_uuid_t value 
// 
TEST_FUNCTION(pal_posix_init_uuid__arg_uuid_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = pal_init_uuid();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_init_uuid unhappy path 
// 
TEST_FUNCTION(pal_posix_init_uuid__neg)
{
    const pal_uuid_t k_uuid_valid = fill_in_here;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_init_uuid(k_uuid_valid);

    // assert    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line40 "pal_misc.h"




// 
// Test pal_uuid_from_string happy path 
// 
TEST_FUNCTION(pal_posix_uuid_from_string__success)
{
    static const char* k_string_valid = fill_in_here;
    const pal_uuid_t k_uuid_valid = fill_in_here;

    // arrange 
    // ... 

    // act 
    result = pal_uuid_from_string(k_string_valid, k_uuid_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_uuid_from_string passing as string argument an invalid const char* value 
// 
TEST_FUNCTION(pal_posix_uuid_from_string__arg_string_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = pal_uuid_from_string();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_uuid_from_string passing as uuid argument an invalid pal_uuid_t value 
// 
TEST_FUNCTION(pal_posix_uuid_from_string__arg_uuid_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = pal_uuid_from_string();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_uuid_from_string unhappy path 
// 
TEST_FUNCTION(pal_posix_uuid_from_string__neg)
{
    static const char* k_string_valid = fill_in_here;
    const pal_uuid_t k_uuid_valid = fill_in_here;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_uuid_from_string(k_string_valid, k_uuid_valid);

    // assert    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line48 "pal_misc.h"





// 
// Test pal_uuid_to_string happy path 
// 
TEST_FUNCTION(pal_posix_uuid_to_string__success)
{
    const pal_uuid_t k_uuid_valid = fill_in_here;
    const char* k_string_valid = fill_in_here;

    // arrange 
    // ... 

    // act 
    result = pal_uuid_to_string(k_uuid_valid, k_string_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_uuid_to_string passing as uuid argument an invalid pal_uuid_t value 
// 
TEST_FUNCTION(pal_posix_uuid_to_string__arg_uuid_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = pal_uuid_to_string();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_uuid_to_string passing as string argument an invalid char* value 
// 
TEST_FUNCTION(pal_posix_uuid_to_string__arg_string_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = pal_uuid_to_string();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_uuid_to_string unhappy path 
// 
TEST_FUNCTION(pal_posix_uuid_to_string__neg)
{
    const pal_uuid_t k_uuid_valid = fill_in_here;
    const char* k_string_valid = fill_in_here;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_uuid_to_string(k_uuid_valid, k_string_valid);

    // assert    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line57 "pal_misc.h"




// 
// Test pal_create_full_path happy path 
// 
TEST_FUNCTION(pal_posix_create_full_path__success)
{
    static const char* k_file_name_valid = fill_in_here;

    // arrange 
    // ... 

    // act 
    result = pal_create_full_path(k_file_name_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_create_full_path passing as file_name argument an invalid const char* value 
// 
TEST_FUNCTION(pal_posix_create_full_path__arg_file_name_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = pal_create_full_path();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_create_full_path unhappy path 
// 
TEST_FUNCTION(pal_posix_create_full_path__neg)
{
    static const char* k_file_name_valid = fill_in_here;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_create_full_path(k_file_name_valid);

    // assert    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line64 "pal_misc.h"




// 
// Test pal_free_path happy path 
// 
TEST_FUNCTION(pal_posix_free_path__success)
{
    static const char* k_path_valid = fill_in_here;

    // arrange 
    // ... 

    // act 
    result = pal_free_path(k_path_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_free_path passing as path argument an invalid const char* value 
// 
TEST_FUNCTION(pal_posix_free_path__arg_path_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = pal_free_path();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_free_path unhappy path 
// 
TEST_FUNCTION(pal_posix_free_path__neg)
{
    static const char* k_path_valid = fill_in_here;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_free_path(k_path_valid);

    // assert    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line71 "pal_misc.h"




// 
// Test pal_set_working_dir happy path 
// 
TEST_FUNCTION(pal_posix_set_working_dir__success)
{
    static const char* k_dir_valid = fill_in_here;

    // arrange 
    // ... 

    // act 
    result = pal_set_working_dir(k_dir_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_set_working_dir passing as dir argument an invalid const char* value 
// 
TEST_FUNCTION(pal_posix_set_working_dir__arg_dir_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = pal_set_working_dir();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_set_working_dir unhappy path 
// 
TEST_FUNCTION(pal_posix_set_working_dir__neg)
{
    static const char* k_dir_valid = fill_in_here;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_set_working_dir(k_dir_valid);

    // assert    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line78 "pal_misc.h"




// 
// Test pal_get_env_var happy path 
// 
TEST_FUNCTION(pal_posix_get_env_var__success)
{
    static const char* k_env_var_valid = fill_in_here;

    // arrange 
    // ... 

    // act 
    result = pal_get_env_var(k_env_var_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_get_env_var passing as env_var argument an invalid const char* value 
// 
TEST_FUNCTION(pal_posix_get_env_var__arg_env_var_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = pal_get_env_var();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_get_env_var unhappy path 
// 
TEST_FUNCTION(pal_posix_get_env_var__neg)
{
    static const char* k_env_var_valid = fill_in_here;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_get_env_var(k_env_var_valid);

    // assert    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line85 "pal_misc.h"




// 
// Test pal_set_env_var happy path 
// 
TEST_FUNCTION(pal_posix_set_env_var__success)
{
    static const char* k_env_var_valid = fill_in_here;
    static const char* k_val_valid = fill_in_here;

    // arrange 
    // ... 

    // act 
    result = pal_set_env_var(k_env_var_valid, k_val_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_set_env_var passing as env_var argument an invalid const char* value 
// 
TEST_FUNCTION(pal_posix_set_env_var__arg_env_var_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = pal_set_env_var();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_set_env_var passing as val argument an invalid const char* value 
// 
TEST_FUNCTION(pal_posix_set_env_var__arg_val_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = pal_set_env_var();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_set_env_var unhappy path 
// 
TEST_FUNCTION(pal_posix_set_env_var__neg)
{
    static const char* k_env_var_valid = fill_in_here;
    static const char* k_val_valid = fill_in_here;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_set_env_var(k_env_var_valid, k_val_valid);

    // assert    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line93 "pal_misc.h"




// 
// Test pal_os_last_error_as_pi_error happy path 
// 
TEST_FUNCTION(pal_posix_get_pi_error__success)
{

    // arrange 
    // ... 

    // act 
    result = pal_os_last_error_as_pi_error();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_os_last_error_as_pi_error unhappy path 
// 
TEST_FUNCTION(pal_posix_get_pi_error__neg)
{

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_os_last_error_as_pi_error();

    // assert    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line100 "pal_misc.h"




// 
// Test pal_os_set_error_as_pi_error happy path 
// 
TEST_FUNCTION(pal_posix_set_pi_error__success)
{
    const int32_t k_error_valid = fill_in_here;

    // arrange 
    // ... 

    // act 
    result = pal_os_set_error_as_pi_error(k_error_valid);

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_os_set_error_as_pi_error passing as error argument an invalid int32_t value 
// 
TEST_FUNCTION(pal_posix_set_pi_error__arg_error_invalid)
{
    // ... 

    // arrange 
    // ... 

    // act 
    result = pal_os_set_error_as_pi_error();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_os_set_error_as_pi_error unhappy path 
// 
TEST_FUNCTION(pal_posix_set_pi_error__neg)
{
    const int32_t k_error_valid = fill_in_here;

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_os_set_error_as_pi_error(k_error_valid);

    // assert    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

;
// line107 "pal_misc.h"




// 
// Test pal_deinit happy path 
// 
TEST_FUNCTION(pal_posix_deinit__success)
{

    // arrange 
    // ... 

    // act 
    result = pal_deinit();

    // assert 
    // ... 
    ASSERT_EXPECTED_CALLS();
}

// 
// Test pal_deinit unhappy path 
// 
TEST_FUNCTION(pal_posix_deinit__neg)
{

    // arrange
    UMOCK_C_NEGATIVE_TESTS_ARRANGE();
    // ...

    // act
    UMOCK_C_NEGATIVE_TESTS_ACT();
    result = pal_deinit();

    // assert    UMOCK_C_NEGATIVE_TESTS_ASSERT(void_ptr, 0, result);

}

#endif

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

