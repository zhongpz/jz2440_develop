CFLAGS 	:= -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer -ffreestanding
uart.bin : start.S  uart.c s3c2440.h uart.h
	arm-linux-gcc $(CFLAGS) -c -o start.o start.S
	arm-linux-gcc $(CFLAGS) -c -o uart.o uart.c
	arm-linux-ld -Ttext 0x0000000 start.o uart.o -o leds_elf
#	arm-linux-ld -Tleds.lds  start.o uart.o -o leds_elf
	arm-linux-objcopy -O binary -S leds_elf uart.bin
	arm-linux-objdump -D -m arm  leds_elf > uart.dis
clean:
	rm -f   uart.dis uart.bin leds_elf *.o
