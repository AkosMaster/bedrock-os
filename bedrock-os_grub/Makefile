C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c libc/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h libc/*.h)
# Nice syntax for file extension replacement
OBJ = ${C_SOURCES:.c=.o cpu/interrupt.o}

# Change this if your cross-compiler is somewhere else
CC = /usr/bin/i386elfgcc/bin/i386-elf-gcc
GDB = /usr/bin/i386elfgcc/bin/i386-elf-gdb
LD = /usr/bin/i386elfgcc/bin/i386-elf-ld

# -g: Use debugging symbols in gcc
CFLAGS = -g -std=gnu99

# First rule is run by default (boot/bootsect.bin)
os-image.bin: kernel.bin
	cat $^ > os-image.bin

# '--oformat binary' deletes all symbols as a collateral, so we don't need
# to 'strip' them manually on this case
kernel.bin: boot/kernel_entry.o ${OBJ}
	${LD} -o $@ -T linker.ld $^ #--oformat binary

# Used for debugging purposes
#kernel.elf: boot/kernel_entry.o ${OBJ}
#	${LD} -o $@ -Ttext 0xC0001000 $^ 

run:
	qemu-system-i386 -hdd os-image.iso -cdrom fs.img -m 2G

# Open the connection to qemu and load our kernel-object file with symbols
debug: os-image.bin kernel.elf
	qemu-system-i386 -s -fda os-image.bin &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

# Generic rules for wildcards
# To make an object, always compile from its .c
%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -ffreestanding -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.bin: %.asm
	nasm $< -f bin -o $@

clean:
	rm -rf *.bin *.dis *.o os-image.bin *.elf
	rm -rf kernel/*.o boot/*.bin drivers/*.o boot/*.o cpu/*.o libc/*.o isodir/*.bin
