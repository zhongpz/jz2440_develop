
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

static struct fb_var_screeninfo fb_var; //����fb_var_screeninfo�ṹ����������ڴ�Ż�ȡ��lcd����
int line_width;    //һ�еĴ�С(�ֽ�)
int pixel_width;    //һ�����صĴ�С
int screen_size;    //��Ļ�Ĵ�С(framebuff��С)
unsigned char *fb_base = NULL;  //mmap()���ص�ַ����framebuff��ʼ��ַ



/************************************************************
 * ��������: compute_string_bbox
 * ��������: ����һ���ֵ����
 * �������: face�����ļ���wstr���ַ����\
 * �������: abbox����С
 * �� �� ֵ: 
 * �޸�����      �汾��      �޸���       �޸�����
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

	/* ��ʼ�� */
	bbox.xMin = bbox.yMin = 32000;
	bbox.xMax = bbox.yMax = -32000;

	/* ָ��ԭ�� */
	pen.x = 0;
	pen.y = 0;

	/* ����ÿ���ַ������ 
	 * ��translate,��load char�Ϳ��Եõ����������
	 */
	for(i = 0; i < wcslen(wstr); i++)
	{
		/* ת��������ԭ���ת���Ƕ� */
		FT_Set_Transform(face, 0, &pen);

		/* ����λͼ���õ��µ�glyph��λͼ�����������slot�� */
		error = FT_Load_Char(face, wstr[i], FT_LOAD_RENDER);
		if(error)
		{
			printf("FT_Load_Char error\n");
			return -1;
		}

		/* ȡ���µ�glyph */
		error = FT_Get_Glyph(face->glyph, &glyph);
		if(error)
		{
			printf("FT_Get_Glyph error\n");
			return -1;
		}

		/* ��glyph�õ�bbox */
		FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &glyph_bbox);

		/* ������� */
		if ( glyph_bbox.xMin < bbox.xMin )
			bbox.xMin = glyph_bbox.xMin;

		if ( glyph_bbox.yMin < bbox.yMin )
			bbox.yMin = glyph_bbox.yMin;

		if ( glyph_bbox.xMax > bbox.xMax )
			bbox.xMax = glyph_bbox.xMax;

		if ( glyph_bbox.yMax > bbox.yMax )
			bbox.yMax = glyph_bbox.yMax;

		 /* ������һ���ַ���ԭ��: increment pen position */
		 pen.x += slot->advance.x;
		 pen.y += slot->advance.y;
		
	}
	*abbox = bbox;
	
}


/************************************************************
 * ��������: lcd_put_pixel
 * ��������: ����������lcd�����
 * �������: x���꣬y���꣬color��ɫ
 * �������: ��
 * �� �� ֵ: ��
 * �޸�����      �汾��      �޸���       �޸�����
 *------------------------------------------------------------
 *
 *************************************************************/
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



/************************************************************
 * ��������: draw_bitmap
 * ��������: ���ݵõ���λͼ����ʾ���ַ�
 * �������: bitmapλͼ��x���꣬������
 * �������: ��
 * �� �� ֵ: ��
 * �޸�����      �汾��      �޸���       �޸�����
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
 * ��������: display_string
 * ��������: ��ʾһ������
 * �������: face�����ļ���wstr���ַ�����lcd_x,lcd_y����\
 * �������: ��
 * �� �� ֵ: 0
 * �޸�����      �汾��      �޸���       �޸�����
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

	/* ��lcd����ϵת��Ϊ�ѿ�������ϵ */
	int x = lcd_x;
	int y = fb_var.yres - lcd_y;

	/* ������� */
	compute_string_bbox(face, wstr, &bbox);

	/* ����ԭ�� */
	pen.x = (x - bbox.xMin) * 64;  //��λ 1/64 ����
	pen.y = (y - bbox.yMax) * 64;

	/* ����ÿ���ַ� */
	for(i = 0; i < wcslen(wstr); i++)
	{
		/* ת��, ����ԭ���ת���Ƕ� */
		FT_Set_Transform(face, 0, &pen);

		/* ����λͼ */
		error = FT_Load_Char(face, wstr[i], FT_LOAD_RENDER);
		if(error)
		{
			printf("FT_Load_Char error\n");
			return -1;
		}

		/* ��LCD����ʾ��ʹ��lcd���� */
		draw_bitmap(&slot->bitmap, slot->bitmap_left, fb_var.yres - slot->bitmap_top);

		/* ������һ��ԭ�� */
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
		
	}
	
	return 0;
}



int main(int argc, char **argv)
{
	/* 1. ȷ�������ַ�Unicodeֵ
	 * ������ڳ�����ֱ��ʹ��Unicode����ֵ����Ҫʹ�ÿ��ַ�����wchat_t
	 */
	wchar_t *chinese_str = L"LOVE ��";
	FT_Library    library;
	FT_Face       face;
	FT_Vector     pen;
	int font_size = 24;
	int error;
	int lcd_x, lcd_y;           
	int fd_fb;

	/* ʹ�÷�����ʾ */
	if(argc < 4)
	{
		printf("Usage : %s <font_file> <lcd_x> <lcd_y> [font_size]\n", argv[0]);	
		return -1;
	}

	/* ʹ�ô����lcd_x �� lcd_y*/
	lcd_x = strtoul(argv[2], NULL, 0);
	lcd_y = strtoul(argv[3], NULL, 0);
	
	/* ʹ�ô���������С */
	if(argc == 5)
	{
		font_size = strtoul(argv[4], NULL, 0);  //�ַ���תΪ�޷���10���Ƴ�����
	}

	/* LCD����س�ʼ�� */
	fd_fb = open("/dev/fb0",O_RDWR);   //�ɶ���д
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

	/* ������ȫ����Ϊ��ɫ */
	memset(fb_base, 0x0, screen_size);	

	/* 2.ʹ��freetype */
	/* 2.1��ʼ��freetype�ֿ� */
	error = FT_Init_FreeType(&library);

	/* 2.2���������ļ� */
	error = FT_New_Face(library, argv[1], 0, &face); //���غ�������ļ������face����

	/* 2.3���������С */
	FT_Set_Pixel_Sizes(face, font_size, 0);

	/* 2.4��ʾһ������ */
	display_string(face, chinese_str, lcd_x, lcd_y);

	munmap(fb_base, screen_size);
	close(fd_fb);
	
	return 0;
}


































