
#include "s3c2440.h"
#include "uart.h"


//sdram内存控制寄存器的初始化
void sdram_init(void)
{
	BWSCON = 0x22000000;  //位宽和等待控制寄存器
	BANKCON6 = 0x18001;   //bank6控制寄存器器
	REFRESH = 0x8404F5;  //刷新控制寄存器
	BANKSIZE = 0xB1;     //bank大小设置寄存器
	MRSRB6 = 0x20;        //sdram模式
}

void led_test(void)
{
	GPFCON &= ~((0x3 << (2*4)) | (0x3 << (2*5)) | (0x3 << (2*6)));
	GPFCON |= ((0x1 << (2*4)) | (0x1 << (2*5)) | (0x1 << (2*6)));
	
	GPFDAT &= ~((0X1 << 4) | (0X1 << 5) | (0X1 << 6));
}


void printHex(unsigned int val)
{
	int i;
	unsigned char arr[8];
	for(i=0;i<8;i++)
	{
		arr[i] = val & 0xf;
		val >>= 4;
	}
	puts("0x");
	for(i=7;i>=0;i--)
	{
		if((arr[i] >= 0) && (arr[i] <= 9))
			putchar(arr[i] + '0');
		if((arr[i] >= 0xA) && (arr[i] <= 0xF))
			putchar(arr[i] - 0xA + 'A');
	}
}

void delay(int num)
{
	while(num--);
}

char g_char1 = 'A';
const char g_char2 = 'B';
unsigned int g_a = 0;
unsigned int g_b;

int main(void)
{
	uart0_init();
	puts("be redy\n\r");
	puts("g_a=");
	printHex(g_a);
	return 0;
}
