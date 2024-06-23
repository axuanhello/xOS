echo Executing gdbinit...\n
echo target remote localhost:1234\n
target remote localhost:1234
echo set architecture i8086\n
set architecture i8086
set print pretty on
#echo display/i $cs*16+$pc
#display/i $cs*16+$pc
echo b *0x7c00\n
b *0x7c00
echo c\n
c
add-symbol-file build/kernel.elf
set architecture i386
b main
c
define seei
    if $cs==8
        set architecture i386
        x/i $pc
    else
        set architecture i8086
        x/i $cs*16+$pc
    end
end