
#if !defined(__decl_test)
#define __decl_test(name, arg, usage) \
    extern int main_##name (int, char**);
#endif

__decl_test(ns, arg, "Device registry tools.");
