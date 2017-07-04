
#if !defined(__decl_test)
#define __decl_test(name, arg, usage) \
    extern int main_##name (int, char**);
#endif

__decl_test(ns, arg, "Device registry tools.");
__decl_test(sd, arg, "Service discovery tools.");
__decl_test(sr, arg, "Name service resolve tools.");