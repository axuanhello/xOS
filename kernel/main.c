#include"mmu.h"
//struct segdescr gdt[256];
void main(void) {
    char message[] = "Hello kernel!";
    char* video = 0xc00b8000;
    for (int i = 0;i < 80 * 25;++i) {
        *video = 0;
        video += 2;
    }
    video=0xc00b8036;
    for (int i = 0;i < sizeof message;++i) {
        *video = message[i];
        video += 2;
    }
    asm("hlt");
}