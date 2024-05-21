target remote localhost:1234
set architecture i8086
display/i $cs*16+$pc
b *0x7c00
c