#ifndef __BITMAP_H
#define __BITMAP_H
#include "types.h"
struct bitmap {
    int byte_size;//总共的字节数
    unsigned char* map;
};
//BUG!没乘8！sizeof得到的是字节数，不是位数
//将bitmap的第i_bit位置1
#define set_bitmap(_bitmap, i_bit) (_bitmap).map[(i_bit) / (8*sizeof((_bitmap).map[0]))]|=1<<((i_bit) % (8*sizeof((_bitmap).map[0])))
#define clear_bitmap(_bitmap, i_bit) (_bitmap).map[(i_bit) / (8*sizeof((_bitmap).map[0]))]&=~(1<<((i_bit) % (8*sizeof((_bitmap).map[0]))))
//判断bitmap的第i_bit位是否为1
#define test_bitmap(_bitmap,i_bit) ((_bitmap).map[(i_bit) / (8*sizeof((_bitmap).map[0]))]&(1<<((i_bit) % (8*sizeof((_bitmap).map[0])))))

long long get_zero_bits(struct bitmap* bitmap, size_t need);
long long get_zero_bit(struct bitmap* bitmap);

#endif