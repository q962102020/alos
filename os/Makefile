TARGAT:=alos
OBJS:=./init/main.o
LIBS:=	./mm/libmm.a					\
		./fs/fat/libfat.a				\
		./kernel/libkernel.a			\
		./fs/ext2/libext2.a				\
		./fs/libfs.a					\
		./drivers/block/libblock.a		\
		./drivers/char/libchar.a		\
		./lib/libsys.a					\
		./drivers/cmos/libcmos.a
				
.PHONY : $(LIBS)
.PHONY : $(OBJS)

					
TEXT_BASE:=0xc0001000 
LDFLAG:=-Ttext $(TEXT_BASE) -T os.lds 
DEV		= ../1vmdisk/1-flat.vmdk 
#DEV:=/dev/sdb
DIR:=include

TOPDIR := $(shell pwd)
CFLAG:=-I $(TOPDIR)/include/  -fno-builtin -fno-asynchronous-unwind-tables -fno-stack-protector
ARFLAG:=rcs
CC:=cc
AR:=ar
LD:=ld
export CFLAG ARFLAG CC AR

	
$(TARGAT):$(OBJS) $(LIBS) 
	ld $(LDFLAG) -nostartfiles -e start_kernel \
		-o $@.elf $^
	objcopy -O binary  -S $@.elf $@
	mount $(DEV) $(DIR)
	cp $(TARGAT) $(DIR)
	umount $(DIR)

$(LIBS) $(OBJS):
	make -C $(dir $@)


.PHONY : clean
clean :
	find .  -name '*.[a,o]'  |xargs  rm -f
	rm -f alos.elf alos
