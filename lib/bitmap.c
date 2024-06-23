#include "bitmap.h"
//寻找need个连续空位，成功返回开始位，失败返回-1。花了比较多时间，非常糟糕的实现:(
long long get_zero_bits(struct bitmap* bitmap, size_t need) {
    //先按字节粒度找第一个有空闲位的字节
    int i_byte = 0;
    long long i_bit = -1;
    size_t sum = 0;
    size_t total_bits = bitmap->byte_size * 8;
find_byte_with_zero:
    //失败时i_byte已被设为i_bit/8
    for (;(i_byte < bitmap->byte_size) && (!(bitmap->map[i_byte] ^ 0xff));++i_byte);

    if (i_byte < bitmap->byte_size) {
        //如果还在同一字节，则不应该设i_bit为该字节第一位。否则可能会出现循环在一个字节中
        if (i_bit / 8 == i_byte) {
            //对于有符号，-1/8=0;
            i_bit = i_bit + 1;//最开始i_bit应为-1，加一后为0。
        }
        else {
            //32位赋值64位只能赋低32位？初始为-1时只能清低32为0？
            i_bit = (long long )i_byte * 8;
        }
        //再按位粒度找i_bit开始（含）的第一个空闲位
        for (;(i_bit < total_bits) && test_bitmap(*bitmap, i_bit);++i_bit);
        if (i_bit >= total_bits) {
            return -1;
        }
        //寻找连续的空闲位
        sum = 1;
        ++i_bit;//i_bit为下一个要被扫描的位
        //for (;sum < need && i_bit < total_bits && !test_bitmap(*bitmap, i_bit);++i_bit, ++sum);
        while (sum < need) {
            if (i_bit < total_bits) {
                //是空闲
                if (!test_bitmap(*bitmap, i_bit)) {
                    ++sum;
                    ++i_bit;
                }
                //把for改用while后缺了break，一直死循环！
                else {
                    break;
                }
            }
            else {
                break;
            }
        }
        if (sum == need) {
            //此时i_bit为最后一个需要的空闲位的后一位
            for (size_t i = i_bit - need;i < i_bit;++i) {
                set_bitmap(*bitmap, i);
            }
            return i_bit-need;
        }
        else if (i_bit < total_bits) {
            //寻找连续区间失败时，i_bit位必为1，且所在字节（不含）之前已无可能有连续区间。
            i_byte = i_bit / 8;
            //重新按字节粒度寻找。
            goto find_byte_with_zero;
        }
        else {
            return -1;
        }
    }
    else {
        return -1;
    }
}
//寻找1个空位。减少不必要的判断。失败返回-1
long long get_zero_bit(struct bitmap* bitmap) {
    //按字节粒度找第一个有空闲位的字节
    int i_byte = 0;
    for (;(i_byte < bitmap->byte_size) && !(bitmap->map[i_byte] ^ 0xff);++i_byte);
    long long i_bit = (long long)i_byte * 8;
    //按位粒度找
    if (i_byte < bitmap->byte_size) {
        for (;test_bitmap(*bitmap, i_bit);++i_bit);
        set_bitmap(*bitmap, i_bit);
        return i_bit;
    }
    else {
        return -1;
    }  
}
