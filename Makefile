CFLAGS= -g -c -O0 -m32 -MMD -fno-pic -fno-pie -fno-stack-protector -fno-builtin -nostdlib -nostdinc -nodefaultlibs -nostartfiles

BUILD=./build
IMAGE=./image
#
ENTRYPOINT=0x7c00

LDFLAGS=-m elf_i386 -Ttext $(ENTRYPOINT) 
LDBIN=--oformat binary

.PHONY:all
all: $(BUILD) $(IMAGE) imagefile
#gcc生成头文件依赖关系文件
-include */*.d
$(BUILD):
	mkdir $(BUILD)
$(IMAGE):
	mkdir $(IMAGE)

imagefile:$(BUILD)/os.bin

$(BUILD)/os.bin:boot/boot.S
#--32不可少，as生成目标文件默认为elf格式
	as  --32 boot/boot.S -o $(BUILD)/boot.o
	ld  $(LDFLAGS)  $(BUILD)/boot.o -o $(BUILD)/os.elf
	readelf -a $(BUILD)/os.elf > $(BUILD)/os_elf.txt
#将elf文件中的代码段和数据段拷贝到纯二进制文件中。
	objcopy -O binary $(BUILD)/os.elf $(BUILD)/os.bin
	dd if=$(BUILD)/os.bin of=$(IMAGE)/disk.img conv=notrunc
#反汇编16位代码使用 -m i8086，反汇编32位使用 -m i386
#由于i386 CPU开始运行时处于16位，因此boot代码使用16位应该编译为16汇编，且反汇编时必须指定i8086，gdb调试时也必须使用set architecture i8086
#	objdump -m i386 -Mi8086,suffix -b binary -D $(BUILD)/os.bin > $(BUILD)/os_dis.txt	
	objdump -m i386 -Mi8086,suffix -D $(BUILD)/os.elf > $(BUILD)/os_elf_dis.txt

.PHONY:clean
clean:
	rm -rf $(BUILD)/
	rm -rf $(IMAGE)/