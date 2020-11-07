
#include "s3c2440.h"
#include "uart.h"

/*sdram内存控制寄存器的初始化*/
void sdram_init(void)
{
	BWSCON = 0x22000000;  //位宽和等待控制寄存器
	BANKCON6 = 0x18001;   //bank6控制寄存器器
	REFRESH = 0x8404F5;  //刷新控制寄存器
	BANKSIZE = 0xB1;     //bank大小设置寄存器
	MRSRB6 = 0x20;        //sdram模式
}

int sdram_test(void)
{
	volatile unsigned long *p = (volatile unsigned long *)0x30000000;  //SDRAM起始地址
	int i;
	for(i=0;i<1000000;i++)
	{
		p[i] = 0x55;
	}
	for(i=0;i<1000000;i++)
	{
		if(p[i] != 0x55)
			return -1;
	}
	return 0;
}
void led_test(void)
{
	GPFCON &= ~((0x3 << (2*4)) | (0x3 << (2*5)) | (0x3 << (2*6)));
	GPFCON |= ((0x1 << (2*4)) | (0x1 << (2*5)) | (0x1 << (2*6)));
	
	GPFDAT &= ~((0X1 << 4) | (0X1 << 5) | (0X1 << 6));
}

int main(void)
{
	uart0_init();
	puts("be redy\n\r");
	sdram_init();
	if(sdram_test() == 0)
	{
		puts("sdram ok!\n\r");
		led_test();
	}
	return 0;
}
