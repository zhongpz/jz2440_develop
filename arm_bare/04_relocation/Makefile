CFLAGS 	:= -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer -ffreestanding
relocation.bin : start.S  relocation.c uart.c s3c2440.h uart.h 
	arm-linux-gcc $(CFLAGS) -c -o start.o start.S
	arm-linux-gcc $(CFLAGS) -c -o uart.o uart.c
	arm-linux-gcc $(CFLAGS) -c -o relocation.o relocation.c
#	arm-linux-ld -Ttext 0x0000000 -Tdata 0x30000000 start.o relocation.o uart.o -o relocation_elf
	arm-linux-ld -Tsdram.lds  start.o relocation.o uart.o -o relocation_elf
	arm-linux-objcopy -O binary -S relocation_elf relocation.bin
	arm-linux-objdump -D -m arm  relocation_elf > relocation.dis
clean:
	rm -f   relocation.dis relocation.bin relocation_elf *.o
