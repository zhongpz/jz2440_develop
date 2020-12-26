
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <linux/fb.h>
#include <linux/font.h>
#include "font_8x16.h"   //添加点阵文件


/*open
 *ioctl
 *mmap
 */
static struct fb_var_screeninfo fb_var; //定义fb_var_screeninfo结构体变量，用于存放获取的lcd参数
int line_width;    //一行的大小(字节)
int pixel_width;    //一个像素的大小
int screen_size;    //屏幕的大小(framebuff大小)
unsigned char *fb_base = NULL;  //mmap()返回地址，即framebuff起始地址


/****************************************************
 * 函数功能: 描点函数，
 * 输入参数: lcd屏的(x,y)坐标和颜色,color格式是0x00RRGGBB
 * 输出参数:
 * 返回值:
 ****************************************************/
void lcd_put_pixel(int x, int y, unsigned int color)
{
	/* 对于不同的bpp格式，写入framebuff要做修改 */
	unsigned char *pen_8 = fb_base+y*line_width+x*pixel_width;  //像素(x,y)对应的framebuff地址
 	unsigned short *pen_16;
 	unsigned int *pen_32;

	unsigned int red, green, blue;

	pen_16 = (unsigned short *)pen_8;
 	pen_32 = (unsigned int *)pen_8;

	switch (fb_var.bits_per_pixel)
  	{
  		/* 8bpp则color表示调色板的值 */
	 	case 8:
	 	{
	 		*pen_8 = color;
	 		break;
		}
		/* 把 red、 green、 blue 这三种 8 位颜色值，根据 RGB565 的格式，
		 * 只保留 red 中的高 5 位、green 中的高 6 位、 blue 中的高 5 位，
		 * 组合成一个新的 16 位颜色值 
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
		/* 32bpp格式一样 */
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
 * 函数功能: 显示ascii码字符
 * 输入参数: lcd屏的(x,y)坐标和字符,
 * 输出参数:
 * 返回值:
 ****************************************************/
void lcd_put_ascii(int x, int y, unsigned char c)
{
	int i, b;
	unsigned char byte;	
	
	/* 获取点阵 */
	unsigned char *dots = (unsigned char *)&fontdata_8x16[c*16];
	
	/* 描点 */
	for(i = 0; i < 16; i++)
	{
		/*每行*/
		byte = dots[i];
		for(b = 7; b >= 0; b--)
		{
			/*每个像素点*/
			if(byte & (1 << b))
			{
				/*显示*/
				lcd_put_pixel(x+7-b, y+i, 0xffffff);
			}
			else
			{	
				/*熄灭*/
				lcd_put_pixel(x+7-b, y+i, 0x0);
			}
		}
	}
	
}


int main(int argc, char **argv)
{
	int fd_fb;
		
	/* 1.open()打开设备 */
	fd_fb = open("/dev/fb0",O_RDWR);   //可读可写
	if(fd_fb < 0)
	{
		printf("can't open /dev/fb0\n");
		return -1;
	}

	/* 2.ioctl()获取lcd参数，成功返回0，失败-1
	 * 驱动中有可变参数和固定参数，我们关心可变参数
	 * 可变参数结构体为 fb_var_screeninfo类型，
	 * FBIOGET_VSCREENINFO命令用于获取lcd可变参数，存放在fb_var中
	 */
	if(ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var))   
	{
		printf("can't get fb_var\n");
		return -1;
	}
	
	/* 3.mmap()映射framebuff 
	 * 要映射内存，需要知道它的地址--这由驱动程序设定
	 * 需要知道它的大小--这由应用程序决定。
	 * PROT_READ | PROT_WRITE 表示该区域可读、可写； 
	 * MAP_SHARED 表示该区域是共享的， APP 写入数据时，
	 * 会直达驱动程序
	 */
	line_width  = fb_var.xres * fb_var.bits_per_pixel / 8;   //一行的大小(字节)
	pixel_width = fb_var.bits_per_pixel / 8;                 //一个像素的大小
	screen_size = fb_var.xres * fb_var.yres * fb_var.bits_per_pixel / 8;  //屏幕的大小(framebuff大小)

	/* 返回的fb_base就是framebuff的起始地址 */
	fb_base = (unsigned char *)mmap(NULL , screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);
	if(fb_base == (unsigned char *)-1)
	{
		printf("can't mmap\n");
		return -1;
	}

	/* 清屏,全部设置为黑色 */
	memset(fb_base, 0x0, screen_size);

	/* 显示ASCII码字符 */
	lcd_put_ascii(fb_var.xres/2, fb_var.yres/2, 'A');
	

	munmap(fb_base, screen_size);
	close(fd_fb);
	
	return 0;
}
































