#include"print.h"
#include"int.h"
void __attribute__((noreturn)) panic(const char* str) {
    put_str("PANIC!\n");
    put_str(str);
    cli();
    for (;;) {
        asm("hlt");
    }
}