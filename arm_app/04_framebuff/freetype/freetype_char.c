
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include <ft2build.h>
#include <linux/fb.h>
#include <fttypes.h>
#include <freetype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <linux/font.h>

static struct fb_var_screeninfo fb_var; //定义fb_var_screeninfo结构体变量，用于存放获取的lcd参数
int line_width;    //一行的大小(字节)
int pixel_width;    //一个像素的大小
int screen_size;    //屏幕的大小(framebuff大小)
unsigned char *fb_base = NULL;  //mmap()返回地址，即framebuff起始地址


/************************************************************
 * 函数名称: lcd_put_pixel
 * 函数功能: 根据坐标在lcd上描点
 * 输入参数: x坐标，y坐标，color颜色
 * 输出参数: 无
 * 返 回 值: 无
 * 修改日期      版本号      修改人       修改内容
 *------------------------------------------------------------
 *
 *************************************************************/
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



/************************************************************
 * 函数名称: draw_bitmap
 * 函数功能: 根据得到的位图，显示出字符
 * 输入参数: bitmap位图，x坐标，有坐标
 * 输出参数: 无
 * 返 回 值: 无
 * 修改日期      版本号      修改人       修改内容
 *------------------------------------------------------------
 *
 *************************************************************/
void draw_bitmap(FT_Bitmap *bitmap, FT_Int x, FT_Int y)
{
	FT_Int i, j , p, q;
	FT_Int x_max = x + bitmap->width;
	FT_Int y_max = y + bitmap->rows;
	for(j = y, q = 0; j < y_max; j++, q++ )
	{
		for(i = x, p = 0; i < x_max; i++, p++)
		{
			if(i < 0 || j < 0 || i >= fb_var.xres || j >= fb_var.yres)
				continue;
			lcd_put_pixel(i, j, bitmap->buffer[q * bitmap->width +p]);
		}
	}
}


int main(int argc, char **argv)
{
	/* 1. 确定给定字符Unicode值
	 * 如果想在程序中直接使用Unicode编码值，则要使用宽字符类型wchat_t
	 */
	wchar_t *chinese_str = L"钟";
	FT_Library    library;
	FT_Face       face;
	FT_Vector     pen;
	FT_GlyphSlot  slot;
	int font_size = 24;
	int           error;
	int           fd_fb;

	/* 使用方法提示 */
	if(argc < 2)
	{
		printf("Usage : %s <font_file> [font_size]\n", argv[0]);	
		return -1;
	}

	/* 使用传入的字体大小 */
	if(argc == 3)
	{
		font_size = strtoul(argv[2], NULL, 0);  //字符串转为无符号10进制长整形
	}

	/* LCD的相关初始化 */
	fd_fb = open("/dev/fb0",O_RDWR);   //可读可写
	if(fd_fb < 0)
	{
		printf("can't open /dev/fb0\n");
		return -1;
	}
	
	if(ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var))   
	{
		printf("can't get fb_var\n");
		return -1;
	}

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

	/* 清屏，全部设为黑色 */
	memset(fb_base, 0x0, screen_size);	

	/* 2.使用freetype */
	/* 2.1初始化freetype字库 */
	error = FT_Init_FreeType(&library);

	/* 2.2加载字体文件 */
	error = FT_New_Face(library, argv[1], 0, &face); //加载后的字体文件存放在face变量
	slot = face->glyph;   // face 中获得 FT_GlyphSlot，后面的代码中文字的位图就是保存在 FT_GlyphSlot 里

	/* 2.3设置字体大小 */
	FT_Set_Pixel_Sizes(face, font_size, 0);

	/* 2.4加载字符，使用Unicode值从字体文件中加载字符的位图 
	 * 字符的位图被存在 slot->bitmap 里，即 face->glyph->bitmap。
	 */
	error = FT_Load_Char(face, chinese_str[0], FT_LOAD_RENDER);
	if(error)
	{
		printf("FT_Load_Char error\n");
		return -1;
	}

	/* 3.根据位图在lcd上显示出来 */
	draw_bitmap(&slot->bitmap, fb_var.xres/2, fb_var.yres/2);	

	munmap(fb_base, screen_size);
	close(fd_fb);
	
	return 0;
}




















