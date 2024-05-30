OBJCPYFLAGS= -j .text -j .bss -j .rodata -j .data -O binary
CC=gcc
CPPFLAGS=-Iinclude
CFLAGS=-g -c -O0 -m32 -MMD -Iinclude -fno-pic -fno-pie -fno-stack-protector -fno-builtin -nostdlib -nostdinc -nodefaultlibs -nostartfiles

BUILD=./build
IMAGE=./image
ENTRYPOINT=0x7c00

BOOT_LDFLAGS=-m elf_i386 -Ttext $(ENTRYPOINT) 
LDBIN=--oformat binary

.PHONY:all
all: $(BUILD)  imagefile
#gcc生成头文件依赖关系文件
-include */*.d
$(BUILD):
	mkdir $(BUILD)
$(IMAGE):
	mkdir $(IMAGE)
imagefile: $(IMAGE)/disk.img 
$(IMAGE)/disk.img:$(BUILD)/kernel.bin $(BUILD)/boot.bin | $(IMAGE)
#创建空磁盘镜像
	dd if=/dev/zero of=$(IMAGE)/disk.img bs=16M count=1
#写入磁盘数据
	dd if=$(BUILD)/boot.bin of=$(IMAGE)/disk.img conv=notrunc
#跳过目标文件的10个扇区附加
	dd if=$(BUILD)/kernel.bin of=$(IMAGE)/disk.img conv=notrunc  seek=10 bs=512
$(BUILD)/kernel.bin:$(BUILD)/main.o 
	ld  -m elf_i386 -e main -Ttext 0xc0010000 $(BUILD)/main.o  -o $(BUILD)/kernel.elf 
	ld $(LDBIN) -m  elf_i386 -e main -Ttext 0xc0010000 $(BUILD)/main.o  -o $(BUILD)/kernel.bin
	readelf -a $(BUILD)/kernel.elf > $(BUILD)/kernel_elf.txt
	objdump -m i386 -D $(BUILD)/kernel.elf > $(BUILD)/kernel_elf_dis.txt
$(BUILD)/boot.bin:$(BUILD)/boot.elf 
#将elf文件中的代码段和数据段拷贝到纯二进制文件中。与ld加上--format=binary效果等同
	objcopy -O binary $(BUILD)/boot.elf $(BUILD)/boot.bin
#反汇编16位代码使用 -m i8086，反汇编32位使用 -m i386
#由于i386 CPU开始运行时处于16位，因此boot代码使用16位应该编译为16汇编，且反汇编时必须指定i8086，gdb调试时也必须使用set architecture i8086
	objdump -m i386 -Mi8086,suffix -D $(BUILD)/boot.elf > $(BUILD)/boot_elf_dis.txt
$(BUILD)/boot.elf:boot/boot.S
	gcc $(CFLAGS) -o $(BUILD)/boot.o boot/boot.S
	ld  $(BOOT_LDFLAGS)  $(BUILD)/boot.o  -o $(BUILD)/boot.elf
$(BUILD)/main.o:kernel/main.c
	gcc $(CFLAGS) -o $(BUILD)/main.o kernel/main.c
#boot/bootmain.o:boot/bootmain.c
#	gcc $(CFLAGS) boot/bootmain.c -o $(BUILD)/bootmain.o
.PHONY:clean debug run
clean:
	rm -rf $(BUILD)/
	rm -rf $(IMAGE)/
QEMU_FLAGS=-serial stdio
debug:all
	qemu-system-i386  -smp 1 -m 128M -s -S -drive file=image/disk.img,index=0,media=disk,format=raw -monitor stdio
run:all
	qemu-system-i386  -smp 1 -m 128M       -drive file=image/disk.img,index=0,media=disk,format=raw -monitor stdio