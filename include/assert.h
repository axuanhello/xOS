#ifndef __ASSERT_H
#define __ASSERT_H
#include"print.h"

#ifdef NDEBUG

#define assert(condition) asm("nop")

#else 

#define assert(condition) \
if(condition){}\
else {\
put_str("\nAssert failure!\n");\
put_str("In file:");\
put_str(__FILE__);\
put_str("\nLine:");\
put_uint(__LINE__);\
put_str("\nFunctin:");\
put_str(__func__);\
put_str("\nCondition:");\
put_str(#condition);\
asm("cli;\n\t""hlt");\
for (;;);\
}

#endif

#endif