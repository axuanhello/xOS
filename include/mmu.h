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
//g取0，右移应注意防止符号扩展，（struct segdesc)起强制类型转换作用
#define SEGDESC(type,base,lim,dpl) (struct segdesc)\
{  (lim)&0xffff,(unsigned int)(base)&0xffff,((unsigned int)(base)>>16)&0xff,(type), 1\
    (dpl),1,((unsigned int)(lim)>>16)&0xf,0,0,1,0,/*g=0*/(unsigned int)(base)>>24\
}   
//g取1。转换后lim以4k为单位，
#define SEGDESC32(type, base, lim, dpl) (struct segdesc)\
{ ((lim) >> 12) & 0xffff, ((unsigned int)(base) & 0xffff,      \
  (((unsigned int)(base) >> 16) & 0xff, (type), 1, (dpl), 1,       \
  ((unsigned int)(lim) >> 28, 0, 0, 1, 1, /*g=1*/((unsigned int)(base) >> 24 \
}

struct gatedesc {
    unsigned int off_15_0 : 16;
    unsigned int selector : 16;
    unsigned int argc : 5;//仅调用门有效
    unsigned int reserved : 3;
    unsigned int type : 4;
    unsigned int s : 1;
    unsigned int dpl : 2;
    unsigned int p : 1;
    unsigned int off_31_16 : 16;
};

#define SETGATE(gate, istrap, _selector, off, d)    \
{                                                   \
  (gate).off_15_0 = (unsigned int)(off) & 0xffff;   \
  (gate).selector = (_selector);                    \
  (gate).argc = 0;                                  \
  (gate).reserved = 0;                              \
  (gate).type = (istrap) ? 0xf : 0xe;               \
  (gate).s = 0;                                     \
  (gate).dpl = (d);                                 \
  (gate).p = 1;                                     \
  (gate).off_31_16 = (unsigned int)(off) >> 16;     \
}
#endif