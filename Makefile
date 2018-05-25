# Makefile for the simple kernel.
CC	=gcc
AS	=as
LD	=ld
OBJCOPY = objcopy
OBJDUMP = objdump
NM = nm

CFLAGS = -m32 -Wall -O -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin 
LDFLAGS = -m elf_i386

# Add debug symbol
CFLAGS += -g

CFLAGS += -I.
# resolve __stack_chk_fai
CFLAGS += -fno-stack-protector

OBJDIR = .

CPUS ?= 1

CPUS ?= 1

all: boot/boot kernel/system
	dd if=/dev/zero of=$(OBJDIR)/kernel.img count=10000 2>/dev/null
	dd if=$(OBJDIR)/boot/boot of=$(OBJDIR)/kernel.img conv=notrunc 2>/dev/null
	dd if=$(OBJDIR)/kernel/system of=$(OBJDIR)/kernel.img seek=1 conv=notrunc 2>/dev/null

include boot/Makefile
include kernel/Makefile

clean:
	rm -rf $(OBJDIR)/boot/*.o $(OBJDIR)/boot/boot.out $(OBJDIR)/boot/boot $(OBJDIR)/boot/boot.asm
	rm -rf $(OBJDIR)/kernel/*.o $(OBJDIR)/kernel/system* kernel.*
	rm -rf $(OBJDIR)/lib/*.o
	rm -rf $(OBJDIR)/user/*.o
	rm -rf $(OBJDIR)/user/*.asm
	rm -rf $(OBJDIR)/kernel/fs/*.o $(OBJDIR)/kernel/fs/fat/*.o
	rm -rf $(OBJDIR)/kernel/drv/*.o

qemu:
	qemu-system-i386 -hda kernel.img -hdb lab7.img -monitor stdio -smp $(CPUS)

debug:
	qemu-system-i386 -hda kernel.img -hdb lab7.img -monitor stdio -s -S -smp $(CPUS)
