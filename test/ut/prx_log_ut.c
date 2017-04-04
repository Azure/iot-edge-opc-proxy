// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define UNIT_UNDER_TEST prx_log
#include "util_ut.h"

//
// 1. Required mocks
//
#include "util_misc.h"
#include "azure_c_shared_utility/xlogging.h"

#define xlogging_set_log_function xlogging_set_log_function_mock
MOCKABLE_FUNCTION(, void, xlogging_set_log_function_mock, LOGGER_LOG, log_function);

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

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

