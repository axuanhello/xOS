
.global int_entry_addr,sendeoi,ignore_int,ignore_out_int
.text
ignore_int:
    iret
ignore_out_int:
    call sendeoi
    iret
sendeoi:
    movb $0x20,%al
    #向从片发送EOI
    outb %al,$0xa0
    #向主片发送EOI
    outb %al,$0x20
    ret
all_int_ret:
    popl %gs
    popl %fs
    popl %es
    popl %ds
    popal           #bug!达成pushal了
    addl $0x8,%esp  #弹出int_no和error_code
    #jmp .
    iret

.data
int_entry_addr:

.macro int_entry  n:req, error_code=0
.text
entry\n :
#构造完整的intr_stack
#是否有error code
.if \error_code
.else
#没有error_code，直接添加占位符
    pushl $0
.endif
#传入中断号
    pushl $\n
    pushal
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    #传入栈顶值，即intr_stack地址
    pushl %esp
    call int_dispatch
    addl $4,%esp
    jmp all_int_ret
    .data
    .long entry\n 
.endm
int_entry 0
int_entry 1
int_entry 2
int_entry 3
int_entry 4
int_entry 5
int_entry 6
int_entry 7
int_entry 8 1
int_entry 9
int_entry 10 1
int_entry 11 1
int_entry 12 1
int_entry 13 1
int_entry 14 1
int_entry 15 
int_entry 16
int_entry 17 1
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
int_entry 32
int_entry 33
int_entry 34
int_entry 35
int_entry 36
int_entry 37
int_entry 38
int_entry 39
int_entry 40
int_entry 41
int_entry 42
int_entry 43
int_entry 44
int_entry 45
int_entry 46
int_entry 47
int_entry 48
int_entry 49
int_entry 50
int_entry 51
int_entry 52
int_entry 53
int_entry 54
int_entry 55
int_entry 56
int_entry 57
int_entry 58
int_entry 59
int_entry 60
int_entry 61
int_entry 62
int_entry 63
int_entry 64
int_entry 65
int_entry 66
int_entry 67
int_entry 68
int_entry 69
int_entry 70
int_entry 71
int_entry 72
int_entry 73
int_entry 74
int_entry 75
int_entry 76
int_entry 77
int_entry 78
int_entry 79
int_entry 80
int_entry 81
int_entry 82
int_entry 83
int_entry 84
int_entry 85
int_entry 86
int_entry 87
int_entry 88
int_entry 89
int_entry 90
int_entry 91
int_entry 92
int_entry 93
int_entry 94
int_entry 95
int_entry 96
int_entry 97
int_entry 98
int_entry 99

int_entry 100
int_entry 101
int_entry 102
int_entry 103
int_entry 104
int_entry 105
int_entry 106
int_entry 107
int_entry 108 
int_entry 109
int_entry 110 
int_entry 111 
int_entry 112 
int_entry 113 
int_entry 114 
int_entry 115 
int_entry 116
int_entry 117 
int_entry 118
int_entry 119
int_entry 120
int_entry 121
int_entry 122
int_entry 123
int_entry 124
int_entry 125
int_entry 126
int_entry 127
int_entry 128
int_entry 129
int_entry 130
int_entry 131
int_entry 132
int_entry 133
int_entry 134
int_entry 135
int_entry 136
int_entry 137
int_entry 138
int_entry 139
int_entry 140
int_entry 141
int_entry 142
int_entry 143
int_entry 144
int_entry 145
int_entry 146
int_entry 147
int_entry 148
int_entry 149
int_entry 150
int_entry 151
int_entry 152
int_entry 153
int_entry 154
int_entry 155
int_entry 156
int_entry 157
int_entry 158
int_entry 159
int_entry 160
int_entry 161
int_entry 162
int_entry 163
int_entry 164
int_entry 165
int_entry 166
int_entry 167
int_entry 168
int_entry 169
int_entry 170
int_entry 171
int_entry 172
int_entry 173
int_entry 174
int_entry 175
int_entry 176
int_entry 177
int_entry 178
int_entry 179
int_entry 180
int_entry 181
int_entry 182
int_entry 183
int_entry 184
int_entry 185
int_entry 186
int_entry 187
int_entry 188
int_entry 189
int_entry 190
int_entry 191
int_entry 192
int_entry 193
int_entry 194
int_entry 195
int_entry 196
int_entry 197
int_entry 198
int_entry 199

int_entry 200
int_entry 201
int_entry 202
int_entry 203
int_entry 204
int_entry 205
int_entry 206
int_entry 207
int_entry 208 
int_entry 209
int_entry 210 
int_entry 211 
int_entry 212 
int_entry 213 
int_entry 214 
int_entry 215 
int_entry 216
int_entry 217 
int_entry 218
int_entry 219
int_entry 220
int_entry 221
int_entry 222
int_entry 223
int_entry 224
int_entry 225
int_entry 226
int_entry 227
int_entry 228
int_entry 229
int_entry 230
int_entry 231
int_entry 232
int_entry 233
int_entry 234
int_entry 235
int_entry 236
int_entry 237
int_entry 238
int_entry 239
int_entry 240
int_entry 241
int_entry 242
int_entry 243
int_entry 244
int_entry 245
int_entry 246
int_entry 247
int_entry 248
int_entry 249
int_entry 250
int_entry 251
int_entry 252
int_entry 253
int_entry 254
int_entry 255


/**
* 无特权级变化栈：（高到低）
* eflags
* cs
* eip
* (error_code) <--ss:esp
* 
* 有特权级变化：
* ss
* esp
* eflags
* cs
* eip
* (error_code) <--ss:esp 

* 若使用pushad 后增加：
* eax 
* ecx
* edx
* ebx
* esp
* ebp
* esi
* edi
* 
*/
