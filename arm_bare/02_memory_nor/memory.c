
#include "s3c2440.h"
#include "uart.h"

void bank0_tacc_set(int val)
{
	BANKCON0 = val << 8;
}

void led_test(void)
{
	GPFCON &= ~((0x3 << (2*4)) | (0x3 << (2*5)) | (0x3 << (2*6)));
	GPFCON |= ((0x1 << (2*4)) | (0x1 << (2*5)) | (0x1 << (2*6)));
	
	GPFDAT &= ~((0X1 << 4) | (0X1 << 5) | (0X1 << 6));
}

int main(void)
{
	char c;
	uart0_init();
	puts("Enter tacc val:\n\r");
	while(1)
	{
		c = getchar();
		putchar(c);
		if(c >= '0' && c <= '7')
		{
			bank0_tacc_set(c - '0');
			led_test();
		}
		else
		{
			puts("Error, val should between 0-7\n\r");
			puts("Enter tacc val:\n\r");
		}
	}
		
	
}
