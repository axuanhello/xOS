void bootmain(void) {
    char message[13] = "Hello world!";
    char* video = 0xb8000;
    //清屏
    for (int i = 0;i <80*25 ;++i) {
        *video = ' ';
        video += 2;
    }
    asm("nop;\n\t");
    video = 0xb8000;
    asm("nop;\n\t");
    for (int i = 0;i < 13;++i) {
        *video = message[i];
        video += 2;
    }
    for (;;) {
        asm("nop;\n\tnop;\n\t;nop;\n\t");
    }
}