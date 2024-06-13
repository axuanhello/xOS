#include"config.h"
#include"ards.h"
//指向boot获取的ards信息位置，此后可复制到内核栈中。
struct ards* ards_buff = (struct ards*)(0x7c00 + ARDS_BUFF_OFFSET);
//由head.S从boot中复制
short ards_nr;

