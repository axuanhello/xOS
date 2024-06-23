OBJCPYFLAGS= -j .text -j .bss -j .rodata -j .data -O binary
CC=gcc
CPPFLAGS=-Iinclude
CFLAGS=-g -c -O0 -m32 -MMD -Iinclude -fno-pic -fno-pie -fno-stack-protector -fno-builtin -nostdlib -nostdinc -nodefaultlibs -nostartfiles

BOOT_ENTRY=0x7c00

BOOT_LDFLAGS=-m elf_i386 -Ttext $(BOOT_ENTRY) 
LDBIN=--oformat binary

DEVICE_SOURCES=$(wildcard device/*.c)
DEVICE_OBJS=$(patsubst device/%.c,build/device/%.o,$(DEVICE_SOURCES))

LIB_SOURCES=$(wildcard lib/*.c)
LIB_OBJS=$(patsubst lib/%.c,build/lib/%.o,$(LIB_SOURCES))

KERNEL_C_SOURCES=$(wildcard kernel/*.c)
KERNEL_C_OBJS=$(patsubst kernel/%.c,build/kernel/%.o,$(KERNEL_C_SOURCES))

MM_SOURCES=$(wildcard mm/*.c)
MM_OBJS=$(patsubst mm/%.c,build/mm/%.o,$(MM_SOURCES))

KERNEL_ALL_OBJS=build/kernel/head.o  build/kernel/int.o $(KERNEL_C_OBJS) $(DEVICE_OBJS) $(LIB_OBJS) $(MM_OBJS)

KERNEL_ENTRY=0xc0010000
KERNEL_LDFLAGS=-m elf_i386 -e _start -Ttext $(KERNEL_ENTRY)

.PHONY:all
all: build build/boot build/lib build/kernel build/mm build/device image imagefile
#gcc生成头文件依赖关系文件
-include build/*.d build/**/*.d
build:
	mkdir -p build
build/boot:
	mkdir -p build/boot
build/device:
	mkdir -p build/device
build/kernel:
	mkdir -p build/kernel
build/lib:
	mkdir -p build/lib
build/mm:
	mkdir -p build/mm
image:
	mkdir -p image

imagefile: image/disk.img

image/disk.img:build/kernel.bin build/boot.bin | image
#创建空磁盘镜像
	dd if=/dev/zero of=image/disk.img bs=16M count=1
#写入磁盘数据
	dd if=build/boot.bin of=image/disk.img conv=notrunc
#跳过目标文件的10个扇区附加
	dd if=build/kernel.bin of=image/disk.img conv=notrunc  seek=10 bs=512

build/kernel.bin:$(KERNEL_ALL_OBJS)
	ld  -m elf_i386 -e _start -Ttext 0xc0010000 $(KERNEL_ALL_OBJS) -o build/kernel.elf 
#	ld $(LDBIN) -m  elf_i386 -e _start -Ttext 0xc0010000 $(KERNEL_OBJS) -o build/kernel.bin
	objcopy -O binary build/kernel.elf build/kernel.bin
	readelf -a build/kernel.elf > build/kernel_elf.txt
	objdump -m i386 -D build/kernel.elf > build/kernel_elf_dis.txt

build/boot.bin:build/boot/boot.elf 
#将elf文件中的代码段和数据段拷贝到纯二进制文件中。与ld加上--format=binary效果等同
	objcopy -O binary build/boot/boot.elf build/boot.bin
#反汇编16位代码使用 -m i8086，反汇编32位使用 -m i386
#由于i386 CPU开始运行时处于16位，因此boot代码使用16位应该编译为16汇编，且反汇编时必须指定i8086，gdb调试时也必须使用set architecture i8086
	objdump -m i386 -Mi8086,suffix -D build/boot/boot.elf > build/boot/boot_elf_dis.txt
build/boot/boot.elf:boot/boot.S
	gcc $(CFLAGS) -o build/boot/boot.o boot/boot.S
	ld  $(BOOT_LDFLAGS)  build/boot/boot.o  -o build/boot/boot.elf
build/device/%.o:device/%.c
	gcc $(CFLAGS) -o $@ $<
build/kernel/head.o:kernel/head.S
	gcc $(CFLAGS) -o $@ $<
build/kernel/int.o:kernel/int.S
	gcc $(CFLAGS) -o $@ $<
build/kernel/%.o:kernel/%.c
	gcc $(CFLAGS) -o $@ $<
build/lib/%.o:lib/%.c
	gcc $(CFLAGS) -o $@ $<
build/mm/%.o:mm/%.c
	gcc $(CFLAGS) -o $@ $<
#boot/bootmain.o:boot/bootmain.c
#	gcc $(CFLAGS) boot/bootmain.c -o build/bootmain.o
.PHONY:clean debug run
clean:
	rm -rf build/
	rm -rf image/
QEMU_FLAGS=-smp 1 -m 128M -drive file=image/disk.img,index=0,media=disk,format=raw -monitor stdio -no-reboot
debug:all
	qemu-system-i386  -s -S $(QEMU_FLAGS)
run:all
	qemu-system-i386  $(QEMU_FLAGS)