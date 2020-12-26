
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
#include <ftglyph.h>

static struct fb_var_screeninfo fb_var; //定义fb_var_screeninfo结构体变量，用于存放获取的lcd参数
int line_width;    //一行的大小(字节)
int pixel_width;    //一个像素的大小
int screen_size;    //屏幕的大小(framebuff大小)
unsigned char *fb_base = NULL;  //mmap()返回地址，即framebuff起始地址



/************************************************************
 * 函数名称: compute_string_bbox
 * 函数功能: 计算一行字的外框
 * 输入参数: face字体文件，wstr宽字符串
 * 输出参数: abbox外框大小
 * 返 回 值: 
 * 修改日期      版本号      修改人       修改内容
 *------------------------------------------------------------
 *
 *************************************************************/
int compute_string_bbox(FT_Face face, wchar_t *wstr, FT_BBox *abbox)
{
	int i;
	int error;
	FT_BBox bbox;
	FT_BBox glyph_bbox;
	FT_Vector pen;
	FT_Glyph glyph;
	FT_GlyphSlot slot = face->glyph;

	/* 初始化 */
	bbox.xMin = bbox.yMin = 32000;
	bbox.xMax = bbox.yMax = -32000;

	/* 指定原点 */
	pen.x = 0;
	pen.y = 0;

	/* 计算每个字符的外框 
	 * 先translate,再load char就可以得到它的外框了
	 */
	for(i = 0; i < wcslen(wstr); i++)
	{
		/* 转换，设置原点和转换角度 */
		FT_Set_Transform(face, 0, &pen);

		/* 加载位图，得到新的glyph和位图，结果保存在slot中 */
		error = FT_Load_Char(face, wstr[i], FT_LOAD_RENDER);
		if(error)
		{
			printf("FT_Load_Char error\n");
			return -1;
		}

		/* 取出新的glyph */
		error = FT_Get_Glyph(face->glyph, &glyph);
		if(error)
		{
			printf("FT_Get_Glyph error\n");
			return -1;
		}

		/* 从glyph得到bbox */
		FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &glyph_bbox);

		/* 更新外框 */
		if ( glyph_bbox.xMin < bbox.xMin )
			bbox.xMin = glyph_bbox.xMin;

		if ( glyph_bbox.yMin < bbox.yMin )
			bbox.yMin = glyph_bbox.yMin;

		if ( glyph_bbox.xMax > bbox.xMax )
			bbox.xMax = glyph_bbox.xMax;

		if ( glyph_bbox.yMax > bbox.yMax )
			bbox.yMax = glyph_bbox.yMax;

		 /* 计算下一个字符的原点: increment pen position */
		 pen.x += slot->advance.x;
		 pen.y += slot->advance.y;
		
	}
	*abbox = bbox;
	
}


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



/************************************************************
 * 函数名称: display_string
 * 函数功能: 显示一行文字
 * 输入参数: face字体文件，wstr宽字符串，lcd_x,lcd_y坐标
 * 输出参数: 无
 * 返 回 值: 0
 * 修改日期      版本号      修改人       修改内容
 *------------------------------------------------------------
 *
 *************************************************************/
int display_string(FT_Face face, wchar_t *wstr, int lcd_x, int lcd_y)
{
	int i;
	int error;
	FT_BBox bbox;
	FT_Vector pen;
	FT_Glyph glyph;
	FT_GlyphSlot slot = face->glyph;

	/* 把lcd作标系转换为笛卡尔坐标系 */
	int x = lcd_x;
	int y = fb_var.yres - lcd_y;

	/* 计算外框 */
	compute_string_bbox(face, wstr, &bbox);

	/* 反推原点 */
	pen.x = (x - bbox.xMin) * 64;  //单位 1/64 像素
	pen.y = (y - bbox.yMax) * 64;

	/* 处理每个字符 */
	for(i = 0; i < wcslen(wstr); i++)
	{
		/* 转换, 设置原点和转换角度 */
		FT_Set_Transform(face, 0, &pen);

		/* 加载位图 */
		error = FT_Load_Char(face, wstr[i], FT_LOAD_RENDER);
		if(error)
		{
			printf("FT_Load_Char error\n");
			return -1;
		}

		/* 在LCD上显示，使用lcd坐标 */
		draw_bitmap(&slot->bitmap, slot->bitmap_left, fb_var.yres - slot->bitmap_top);

		/* 计算下一个原点 */
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
		
	}
	
	return 0;
}



int main(int argc, char **argv)
{
	/* 1. 确定给定字符Unicode值
	 * 如果想在程序中直接使用Unicode编码值，则要使用宽字符类型wchat_t
	 */
	wchar_t *chinese_str = L"LOVE 婷";
	FT_Library    library;
	FT_Face       face;
	FT_Vector     pen;
	int font_size = 24;
	int error;
	int lcd_x, lcd_y;           
	int fd_fb;

	/* 使用方法提示 */
	if(argc < 4)
	{
		printf("Usage : %s <font_file> <lcd_x> <lcd_y> [font_size]\n", argv[0]);	
		return -1;
	}

	/* 使用传入的lcd_x 和 lcd_y*/
	lcd_x = strtoul(argv[2], NULL, 0);
	lcd_y = strtoul(argv[3], NULL, 0);
	
	/* 使用传入的字体大小 */
	if(argc == 5)
	{
		font_size = strtoul(argv[4], NULL, 0);  //字符串转为无符号10进制长整形
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

	/* 2.3设置字体大小 */
	FT_Set_Pixel_Sizes(face, font_size, 0);

	/* 2.4显示一行文字 */
	display_string(face, chinese_str, lcd_x, lcd_y);

	munmap(fb_base, screen_size);
	close(fd_fb);
	
	return 0;
}


































