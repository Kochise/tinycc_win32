# modify to your linux-2.4.26 kernel path (you must build the kernel first)
KERNEL_PATH=../linux-2.4.26
# modify to your TinyCC 0.9.21 path (you must build TinyCC first)
TCC_PATH=../..
CC=gcc
CFLAGS=-D__KERNEL__ -Wall -O2 -g -I$(KERNEL_PATH)/include -fno-builtin-printf -DCONFIG_TCCBOOT -mpreferred-stack-boundary=2 -march=i386 -falign-functions=0 -I.

all: tccboot initrd.img

#tccboot.user: tcc.o main.o ctype.o vsprintf.o lib.o malloc.o dtoa.o user.o
#	$(CC) -static -nostdlib -o $@ $^

tccboot.out: head.o tcc.o main.o ctype.o vsprintf.o lib.o malloc.o \
             dtoa.o gunzip.o
	ld -e startup_32 -Ttext=0x100000 -N -o $@ $^

tccboot.bin: tccboot.out
	objcopy -O binary -R .note -R .comment -S $< $@

tccboot: tccboot.bin
	$(KERNEL_PATH)/arch/i386/boot/tools/build \
              -b $(KERNEL_PATH)/arch/i386/boot/bbootsect \
              $(KERNEL_PATH)/arch/i386/boot/bsetup \
              $< CURRENT > $@

tcc.o: $(TCC_PATH)/tcc.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) -D__ASSEMBLY__ -D__KERNEL__ -I$(KERNEL_PATH)/include -c -o $@ $<

clean:
	rm -f *~ *.o tccboot.out tccboot.bin example.romfs

cleanall: clean
	rm -f tccboot example.romfs initrd.img

example.romfs: example/boot/tccargs example/hello.c
	cd example ; genromfs -f ../example.romfs

initrd.img: example.romfs
	gzip < $< > $@