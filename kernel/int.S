int_message:
.asciz "Interrupt!"
.global int_start,int_len

.macro int_entry  n

    cld
    #movl $ignore_int-$int_message,%ecx
    xorl %edi,%edi
1:
    movb int_message(%edi),%al
    movb %al,0xb8000(,%edi,2)
    inc %edi
    cmp $0,%al
    jne 1b
    #调用C函数打印中断号
    pushl %edi
    call set_cursor
    sub $4,%esp

    movl $\n ,%eax
    push %eax
    call put_int
    sub $4, %esp

    #xor %al,%al
    #add $48,%al
    #add $\n ,%al
    #mov %al,0xb8000(,%edi,2)
    hlt
.endm
int_start:
int_entry  0

int_entry 1
int_entry 2
int_entry 3
int_entry 4
int_entry 5
int_entry 6
int_entry 7
int_entry 8
int_entry 9
int_entry 10
int_entry 11
int_entry 12
int_entry 13
int_entry 14
int_entry 16
int_entry 17
int_entry 18
int_entry 19
int_entry 20
int_entry 21
int_entry 22
int_entry 23
int_entry 24
int_entry 25
int_entry 26
int_entry 27
int_entry 28
int_entry 29
int_entry 30
int_entry 31
int_label:
int_entry 32
int_len:
    .long . - int_label