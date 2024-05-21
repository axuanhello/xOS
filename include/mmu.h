#ifndef __MMU_H_
#define __MMU_H_

//从低地址到高地址
struct segdesc {
    unsigned int lim_15_0 : 16;
    unsigned int base_15_0 : 16;
    unsigned int base_23_16 : 8;
    unsigned int type : 4;
    unsigned int s : 1;
    unsigned int dpl : 1;
    unsigned int p : 1;
    unsigned int lim_19_16 : 4;
    unsigned int avl : 1;
    unsigned int l : 1;
    unsigned int d_b : 1;
    unsigned int g : 1;
    unsigned int base_31_24 : 8;
};
//g取0，右移应注意防止符号扩展
#define SEGDESC(type,base,lim,dpl) (struct segdesc)\
{  (lim)&0xffff,(base)&0xffff,((unsigned int)(base)>>16)&0xff,type, 1\
    (dpl),1,((unsigned int)(lim)>>16)&0xf,0,0,1,0,(unsigned int)(base)>>24\
}   

#endif