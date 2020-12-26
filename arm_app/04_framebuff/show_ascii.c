
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <linux/fb.h>
#include <linux/font.h>
#include "font_8x16.h"   //��ӵ����ļ�


/*open
 *ioctl
 *mmap
 */
static struct fb_var_screeninfo fb_var; //����fb_var_screeninfo�ṹ����������ڴ�Ż�ȡ��lcd����
int line_width;    //һ�еĴ�С(�ֽ�)
int pixel_width;    //һ�����صĴ�С
int screen_size;    //��Ļ�Ĵ�С(framebuff��С)
unsigned char *fb_base = NULL;  //mmap()���ص�ַ����framebuff��ʼ��ַ


/****************************************************
 * ��������: ��㺯����
 * �������: lcd����(x,y)�������ɫ,color��ʽ��0x00RRGGBB
 * �������:
 * ����ֵ:
 ****************************************************/
void lcd_put_pixel(int x, int y, unsigned int color)
{
	/* ���ڲ�ͬ��bpp��ʽ��д��framebuffҪ���޸� */
	unsigned char *pen_8 = fb_base+y*line_width+x*pixel_width;  //����(x,y)��Ӧ��framebuff��ַ
 	unsigned short *pen_16;
 	unsigned int *pen_32;

	unsigned int red, green, blue;

	pen_16 = (unsigned short *)pen_8;
 	pen_32 = (unsigned int *)pen_8;

	switch (fb_var.bits_per_pixel)
  	{
  		/* 8bpp��color��ʾ��ɫ���ֵ */
	 	case 8:
	 	{
	 		*pen_8 = color;
	 		break;
		}
		/* �� red�� green�� blue ������ 8 λ��ɫֵ������ RGB565 �ĸ�ʽ��
		 * ֻ���� red �еĸ� 5 λ��green �еĸ� 6 λ�� blue �еĸ� 5 λ��
		 * ��ϳ�һ���µ� 16 λ��ɫֵ 
		 */
	 	case 16:
		{
			/* 565 */
			 red = (color >> 16) & 0xff;
			 green = (color >> 8) & 0xff;
			 blue = (color >> 0) & 0xff;
			 color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
			*pen_16 = color;
			 break;
		 }
		/* 32bpp��ʽһ�� */
		case 32:
		{
			 *pen_32 = color;
			 break;
		}
		default:
		{
			 printf("can't surport %dbpp\n", fb_var.bits_per_pixel);
			 break;
		}
	}
}


/****************************************************
 * ��������: ��ʾascii���ַ�
 * �������: lcd����(x,y)������ַ�,
 * �������:
 * ����ֵ:
 ****************************************************/
void lcd_put_ascii(int x, int y, unsigned char c)
{
	int i, b;
	unsigned char byte;	
	
	/* ��ȡ���� */
	unsigned char *dots = (unsigned char *)&fontdata_8x16[c*16];
	
	/* ��� */
	for(i = 0; i < 16; i++)
	{
		/*ÿ��*/
		byte = dots[i];
		for(b = 7; b >= 0; b--)
		{
			/*ÿ�����ص�*/
			if(byte & (1 << b))
			{
				/*��ʾ*/
				lcd_put_pixel(x+7-b, y+i, 0xffffff);
			}
			else
			{	
				/*Ϩ��*/
				lcd_put_pixel(x+7-b, y+i, 0x0);
			}
		}
	}
	
}


int main(int argc, char **argv)
{
	int fd_fb;
		
	/* 1.open()���豸 */
	fd_fb = open("/dev/fb0",O_RDWR);   //�ɶ���д
	if(fd_fb < 0)
	{
		printf("can't open /dev/fb0\n");
		return -1;
	}

	/* 2.ioctl()��ȡlcd�������ɹ�����0��ʧ��-1
	 * �������пɱ�����͹̶����������ǹ��Ŀɱ����
	 * �ɱ�����ṹ��Ϊ fb_var_screeninfo���ͣ�
	 * FBIOGET_VSCREENINFO�������ڻ�ȡlcd�ɱ�����������fb_var��
	 */
	if(ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var))   
	{
		printf("can't get fb_var\n");
		return -1;
	}
	
	/* 3.mmap()ӳ��framebuff 
	 * Ҫӳ���ڴ棬��Ҫ֪�����ĵ�ַ--�������������趨
	 * ��Ҫ֪�����Ĵ�С--����Ӧ�ó��������
	 * PROT_READ | PROT_WRITE ��ʾ������ɶ�����д�� 
	 * MAP_SHARED ��ʾ�������ǹ���ģ� APP д������ʱ��
	 * ��ֱ����������
	 */
	line_width  = fb_var.xres * fb_var.bits_per_pixel / 8;   //һ�еĴ�С(�ֽ�)
	pixel_width = fb_var.bits_per_pixel / 8;                 //һ�����صĴ�С
	screen_size = fb_var.xres * fb_var.yres * fb_var.bits_per_pixel / 8;  //��Ļ�Ĵ�С(framebuff��С)

	/* ���ص�fb_base����framebuff����ʼ��ַ */
	fb_base = (unsigned char *)mmap(NULL , screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);
	if(fb_base == (unsigned char *)-1)
	{
		printf("can't mmap\n");
		return -1;
	}

	/* ����,ȫ������Ϊ��ɫ */
	memset(fb_base, 0x0, screen_size);

	/* ��ʾASCII���ַ� */
	lcd_put_ascii(fb_var.xres/2, fb_var.yres/2, 'A');
	

	munmap(fb_base, screen_size);
	close(fd_fb);
	
	return 0;
}
































