    .global _start
//必须有，指示as编译器接下来的代码生成16位汇编，且源文件不能有32位语法
    .code16

    .text
_start:
    movw $0xb800,%ax
    movw %ax,%ds  
    movb $'H',%ds:0
    movb $'e',%ds:(2)
    movb $'l',%ds:(4)
    movb $'l',%ds:(6)
    movb $'o',%ds:(8)
    movb $0x20,%ds:(10)
    movb $'O',%ds:(12)
    movb $'S',%ds:(14)
    jmp .
    .org 0x1fe
    .byte 0x55,0xaa
