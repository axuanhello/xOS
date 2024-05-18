CFLAGS= -g -c -O0 -m32 -fno-pic -fno-pie -fno-stack-protector -fno-builtin -nostdlib -nostdinc -nodefaultlibs -nostartfiles

BUILD=./build
IMAGE=./image
#生成elf文件所必须的，指明elf将被加载到内存的哪个地址。如果是生成纯binary文件，应该不用了吧？
ENTRYPOINT=0x7c00

LDFLAGS=-m elf_i386 -Ttext $(ENTRYPOINT) 
LDBIN=--oformat binary

.PHONY:all
all: $(BUILD) $(IMAGE) imagefile

$(BUILD):
	mkdir $(BUILD)
$(IMAGE):
	mkdir $(IMAGE)

imagefile:$(BUILD)/os.bin
	dd if=$(BUILD)/os.bin of=$(IMAGE)/disk.img conv=notrunc

$(BUILD)/os.bin:boot/boot.s
#--32不可少
	as  --32 boot/boot.s -o $(BUILD)/boot.o
	ld  $(LDFLAGS)  $(BUILD)/boot.o -o $(BUILD)/os.elf
	readelf -a $(BUILD)/os.elf > $(BUILD)/os_elf.txt
#将elf文件中的代码段和数据段拷贝到纯二进制文件中，此时内存地址是写死的，不能通过OS调度。
	objcopy -O binary $(BUILD)/os.elf $(BUILD)/os.bin
#反汇编16位代码使用 -m i8086，反汇编32位使用 -m i386
#由于i386 CPU开始运行时处于16位，因此boot代码使用16位应该编译为16汇编，且反汇编时必须指定i8086，gdb调试时也必须使用set architecture i8086
	objdump -m i8086 -b binary -D $(BUILD)/os.bin > $(BUILD)/os_dis.txt	
	

.PHONY:clean
clean:
	rm -rf $(BUILD)/
	rm -rf $(IMAGE)/