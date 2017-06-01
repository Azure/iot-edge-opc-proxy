
#include "macro_utils.h"

#define valid_decl(arg_type, arg_name) static const arg_type C3(k_, arg_name, _valid); lr() \

#define valid_arg(count, arg_type, arg_name) C3(k_, arg_name, _valid) IFCOMMA(count)

#define decl_success(r, name, ...) \
cc= lr()\
cc= Test name happy path lr()\
cc= lr()\
TEST_FUNCTION(C2(name, __success)) lr()\
{ lr()\
    FOR_EACH_2(valid_decl, __VA_ARGS__) \
    r result; lr()\
lr()\
    cc= arrange  lr()\
    cc= ... lr()\
lr()\
    cc= act lr()\
    result = name##(FOR_EACH_2_COUNTED(valid_arg, __VA_ARGS__)); lr()\
lr()\
    cc= assert lr()\
    ASSERT_EXPECTED_CALLS(); lr()\
    ASSERT_ARE_EQUAL(r, er_ok, result); lr()\
    cc= ... lr()\
} lr()\
lr()\

#define decl_neg(r, name, ...) \
cc= lr()\
cc= Test name unhappy path lr()\
cc= lr()\
TEST_FUNCTION(C2(name, __neg)) lr()\
{ lr()\
    FOR_EACH_2(valid_decl, __VA_ARGS__) \
    r result; lr()\
lr()\
    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init()); lr()\
lr()\
    cc= arrange lr()\
    UMOCK_C_NEGATIVE_TESTS_ARRANGE(); lr()\
    cc= ... lr()\
lr()\
    cc= act lr()\
    UMOCK_C_NEGATIVE_TESTS_ACT(); lr()\
    result = name##(FOR_EACH_2_COUNTED(valid_arg, __VA_ARGS__)); lr()\
lr()\
    cc= assert lr()\
    UMOCK_C_NEGATIVE_TESTS_ASSERT(r, result, er_ok); lr()\
} lr()\
lr()\

#define decl_arg(name, argtype, argname) \
cc= lr()\
cc= Test name passing as argname argument an invalid argtype value lr()\
cc= lr()\
TEST_FUNCTION(C4(name, __arg_, argname, _invalid)) lr()\
{ lr()\
    cc= ... lr()\
    int32_t result; lr()\
lr()\
    cc= arrange  lr()\
    cc= ... lr()\
lr()\
    cc= act lr()\
    handle = name##(); lr()\
lr()\
    cc= assert lr()\
    ASSERT_EXPECTED_CALLS(); lr()\
    ASSERT_ARE_EQUAL(int32_t, er_fault, result); lr()\
    cc= ... lr()\
} lr()\
lr()\

#define DECL_F_CUSTOM(s, r, m, name, ...) \
>>>>>>>>>>>> BEGIN name \
lr()decl_success(r, name, __VA_ARGS__) \
IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(decl_arg, name, __VA_ARGS__), ) \
decl_neg(r, name, __VA_ARGS__) \
>>>>>>>>>>>> END name \
lr()\


#include "pal_sd.h"


