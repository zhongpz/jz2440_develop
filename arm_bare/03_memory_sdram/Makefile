CFLAGS 	:= -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer -ffreestanding
memory.bin : start.S  memory.c uart.c s3c2440.h uart.h
	arm-linux-gcc $(CFLAGS) -c -o start.o start.S
	arm-linux-gcc $(CFLAGS) -c -o uart.o uart.c
	arm-linux-gcc $(CFLAGS) -c -o memory.o memory.c
	arm-linux-ld -Ttext 0x0000000 start.o memory.o uart.o -o leds_elf
#	arm-linux-ld -Tleds.lds  start.o memory.o -o leds_elf
	arm-linux-objcopy -O binary -S leds_elf memory.bin
	arm-linux-objdump -D -m arm  leds_elf > memory.dis
clean:
	rm -f   memory.dis memory.bin leds_elf *.o
