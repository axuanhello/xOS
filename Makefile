CFLAGS= -g -c -O0 -m32 -MMD -Iinclude -fno-pic -fno-pie -fno-stack-protector -fno-builtin -nostdlib -nostdinc -nodefaultlibs -nostartfiles
OBJCPYFLAGS= -j .text -j .bss -j .rodata -j .data -O binary
BUILD=./build
IMAGE=./image
ENTRYPOINT=0x7c00

LDFLAGS=-m elf_i386 -Ttext $(ENTRYPOINT) 
LDBIN=--oformat binary

.PHONY:all
all: $(BUILD)  imagefile
#gcc生成头文件依赖关系文件
-include */*.d
$(BUILD):
	mkdir $(BUILD)
$(IMAGE):
	mkdir $(IMAGE)
imagefile:$(IMAGE) $(IMAGE)/disk.img 
$(IMAGE)/disk.img:$(BUILD)/os.bin
#创建空磁盘镜像
#	qemu-img create -f raw $(IMAGE)/disk.img 128k
	dd if=/dev/zero of=$(IMAGE)/disk.img bs=1M count=1
#写入磁盘数据
	dd if=$(BUILD)/os.bin of=$(IMAGE)/disk.img conv=notrunc
$(BUILD)/os.bin:boot/boot.o boot/bootmain.o
#--32不可少，as生成目标文件默认为elf格式
#	as  --32 boot/boot.S -o $(BUILD)/boot.o
	ld  $(LDFLAGS)  $(BUILD)/boot.o $(BUILD)/bootmain.o -o $(BUILD)/os.elf
	readelf -a $(BUILD)/os.elf > $(BUILD)/os_elf.txt
#将elf文件中的代码段和数据段拷贝到纯二进制文件中。
	objcopy -O binary $(BUILD)/os.elf $(BUILD)/os.bin
#反汇编16位代码使用 -m i8086，反汇编32位使用 -m i386
#由于i386 CPU开始运行时处于16位，因此boot代码使用16位应该编译为16汇编，且反汇编时必须指定i8086，gdb调试时也必须使用set architecture i8086
#	objdump -m i386 -Mi8086,suffix -b binary -D $(BUILD)/os.bin > $(BUILD)/os_dis.txt	
	objdump -m i386 -Mi8086,suffix -D $(BUILD)/os.elf > $(BUILD)/os_elf_dis.txt
boot/boot.o:boot/boot.S
	gcc $(CFLAGS) boot/boot.S -o $(BUILD)/boot.o
boot/bootmain.o:boot/bootmain.c
	gcc $(CFLAGS) boot/bootmain.c -o $(BUILD)/bootmain.o
.PHONY:clean debug run
clean:
	rm -rf $(BUILD)/
	rm -rf $(IMAGE)/
QEMU_FLAGS=-serial stdio
debug:all
	qemu-system-i386  -smp 1 -m 128M -s -S -drive file=image/disk.img,index=0,media=disk,format=raw 
run:all
	qemu-system-i386  -smp 1 -m 128M       -drive file=image/disk.img,index=0,media=disk,format=raw 