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

//
// 5. Teardown tests and test suite
//
DECLARE_TEST_COMPLETE()

