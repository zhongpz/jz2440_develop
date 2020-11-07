
#include "s3c2440.h"
#include "uart.h"

/* 1.串口初始化 */
void uart0_init(void)
{
	/*配置GPH2,3为串口引脚*/
	GPHCON &= ~((0x3 << (2*2)) | (0x3 << (2*3)));
	GPHCON |= ((0x2 << (2*2)) | (0x2 << (2*3)));
	
	/*配置引脚GPH2,3为上拉状态*/
	GPHUP &= ~((0X1 << 2) | (0X1 << 3));
	
	/*设置波特率*/
	UBRDIV0 = 26;
	
	/*设置uart为pclk，中断、查询模式*/
	UCON0 = 0x0005;
	
	/*设置数据格式,无校验位，一个停止位，8位数据*/
	ULCON0 = 0x03;
	
}	
/* 2.发送一个字符 */
void putchar(unsigned char c)
{
	while(!(UTRSTAT0 & (1 << 2)));   //判断发送Buff是否为空，等待为空后向UTXH0写入要发送的内容
	UTXH0 = c;
}

/* 3.接收一个字符 */
int getchar(void)
{
	while(!(UTRSTAT0 & (1 << 0)));   //判断接收Buff是否为空，不为空时读取URXH0内容
	return URXH0;
}

/* 4.发送一串字符 */
void puts(char *s)
{
	while(*s)
	{
		putchar(*s);
		s++;
	}
}


