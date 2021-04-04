
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <linux/fb.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include "font_8x16.h"   //添加点阵头文件



/*open
 *ioctl
 *mmap
 */

#define RED   0xff0000
#define GREEN 0x00ff00
#define BLUE  0x0000ff
#define BLACK 0x000000
#define WHITE 0xffffff
#define PI    3.1415926

/* lcd信息 */
typedef struct lcd_info{
	int xres;
	int yres;
	int line_width;
	int pixel_width;
	int screen_size;
	unsigned short *fb_base;
	int fd_fb;
}LCD_INFO, *pLCD_INFO;

///* 线条信息 */
//typedef struct line_info{
//	int start_x;
//	int start_y;
//	int end_x;
//	int end_y;
//	unsigned int color;
//	unsigned short width = 1;	
//}LINE_INFO, *pLINE_INFO;


///* 圆信息 */
//typedef struct circel_info{
//	int center_x;
//	int center_y;
//	unsigned short radius;
//	unsigned int color;
//	unsigned short width = 1;	
//}CIRCEL_INFO, *pCircel_info;


typedef struct mytime{
	int hour;
	int min;
	int sec;
}MYTIME, *pMYTIME;

/*************************************************************
LCD初始化函数
***************************************************************/
static void lcd_init(pLCD_INFO lcd_info)
{
	static struct fb_var_screeninfo fb_var;

	/* 1.open()打开设备 */
	lcd_info->fd_fb = open("/dev/fb0",O_RDWR);   //可读可写
	if(lcd_info->fd_fb < 0)
	{
		printf("can't open /dev/fb0\n");
	}

	if(ioctl(lcd_info->fd_fb, FBIOGET_VSCREENINFO, &fb_var))   
	{
		printf("can't get fb_var\n");
	}
	lcd_info->xres         = fb_var.xres;
	lcd_info->yres         = fb_var.yres;
	lcd_info->line_width  = fb_var.xres * fb_var.bits_per_pixel / 8;
	lcd_info->pixel_width = fb_var.bits_per_pixel / 8;
	lcd_info->screen_size = fb_var.xres * fb_var.yres * fb_var.bits_per_pixel / 8;
	lcd_info->fb_base     = (unsigned short *)mmap(NULL , lcd_info->screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_info->fd_fb, 0);
	/* 清屏,全部设置为黑色 */
	memset(lcd_info->fb_base, 0x000000, lcd_info->screen_size);
	if(lcd_info->fb_base == (unsigned short *)-1)
	{
		printf("can't mmap\n");
	}
}

/*************************************************************
LCD去初始化
***************************************************************/
static void lcd_del(pLCD_INFO lcd_info)
{
	munmap(lcd_info->fb_base, lcd_info->screen_size);
	close(lcd_info->fd_fb);
}


/*************************************************************
描点函数，参数有lcd屏的x,y和颜色,color格式是0x00RRGGBB,fb_base显存基地址
***************************************************************/
static void put_pixel(int x, int y, unsigned int color, pLCD_INFO lcd_info)
{
	/* 加载显存 */
	unsigned short *paddr = lcd_info->fb_base + lcd_info->xres * y + x;

	/* 565格式 */
	unsigned int red, green, blue;
	red = (color >> 16) & 0xff;
	green = (color >> 8) & 0xff;
	blue = (color >> 0) & 0xff;
	color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
	*paddr = color;
}

/************************************************************
显示单个ASCII字符
***************************************************************/
static void show_ascii(int x, int y, unsigned char c, unsigned int color, pLCD_INFO lcd_info)
{
	int i, b;
	unsigned char byte;	
	
	/* 获取点阵 */
	unsigned char *dots = (unsigned char *)&fontdata_8x16[c*16];
	
	/* 描点 16 x 8 */
	for(i = 0; i < 16; i++)
	{
		byte = dots[i];
		for(b = 7; b >= 0; b--)
		{
			if(byte & (1 << b))
			{
				/*显示*/
				put_pixel(lcd_info->xres/2 + (x+7-b), lcd_info->yres/2 - (y-i), color, lcd_info);
			}
			else
			{	
				/*熄灭*/
				put_pixel(lcd_info->xres/2 + (x+7-b), lcd_info->yres/2 - (y-i), BLACK, lcd_info);
			}
		}
	}
}

/************************************************************
连续显示ASCII字符
***************************************************************/
static void show_text(int x, int y, unsigned short gap, unsigned char *s, unsigned int color, pLCD_INFO lcd_info)
{
	if(gap < 1)  /* 默认间隔 */
		gap = 1;
	
	int pen_x = x; /* 每个字符起点 */
	int pen_y = y; 
	
	while(*s != '\0')
	{
		show_ascii(pen_x, pen_y, *s, color, lcd_info);
		pen_x += gap + 8;
		s++;
	}
}


/*************************************************************
矩形区域填充函数
***************************************************************/
static void fill_rectangle(int start_x, int start_y, int end_x, int end_y, unsigned int color, pLCD_INFO lcd_info)
{
	int min_x, min_y, max_x, max_y;
    /* 从上往下填充 */
    if(start_y < end_y)
    {
        min_x = start_x;
        min_y = start_y;
        max_x = end_x;
        max_y = end_y;
    }
    else
    {
        min_x = end_x;
        min_y = end_y;
        max_x = start_x;
        max_y = start_y;
    }
    int i, j;
    for(i = min_y; i < max_y; i++)  
    {
        for(j = min_x; j < max_x; j++)
        {
            put_pixel(lcd_info->xres/2 + j, lcd_info->yres/2 - i, color, lcd_info);
        }
    }
}

/*************************************************************
水平线刷子
***************************************************************/
static void linebrush_h(int x, int y, unsigned short width, unsigned int color, pLCD_INFO lcd_info)
{
	int i;
	for(i = 0; i < width; i++)
	{
		put_pixel(lcd_info->xres/2 + (x + i), lcd_info->yres/2 - y, color, lcd_info);
		put_pixel(lcd_info->xres/2 + (x - i), lcd_info->yres/2 - y, color, lcd_info);
	}
}


/*************************************************************
垂直线刷子
***************************************************************/
static void linebrush_v(int x, int y, unsigned short width, unsigned int color, pLCD_INFO lcd_info)
{
	int i;
	for(i = 0; i < width; i++)
	{
		put_pixel(lcd_info->xres/2 + x, lcd_info->yres/2 - (y + i), color, lcd_info);
		put_pixel(lcd_info->xres/2 + x, lcd_info->yres/2 - (y - i), color, lcd_info);
	}
}

/*************************************************************
方格线刷子
***************************************************************/
static void linebrush_s(int x, int y, unsigned short width, unsigned int color, pLCD_INFO lcd_info)
{
	fill_rectangle(x - (width - 1), y - (width - 1), x + (width -1), y + (width - 1), color, lcd_info);
}

/*************************************************************
画线函数
***************************************************************/
static void draw_line(int start_x, int start_y, int end_x, int end_y,
	unsigned short width, unsigned int color, pLCD_INFO lcd_info)
{
	/* Bresenham算法 */

	if(width < 1)  //默认宽度
		width =1;
	
    int x, y, delta_x, delta_y, d, i;
    int min_x, max_x, min_y, max_y;
    delta_x = end_x - start_x;
    delta_y = end_y - start_y;
    if(abs(delta_x) > abs(delta_y))  /* 斜率绝对值在（0,1），步进方向为x轴 */
    {
        /* 默认画点从左往右 */
        if(start_x < end_x)
        {
            x = start_x;
            y = start_y;
            min_x = start_x;
            max_x = end_x;
        }
        else
        {
            x = end_x;
            y = end_y;     
            min_x = end_x;
            max_x = start_x;
        }
		//linebrush_v(x, y, width, color, lcd_info);
		//fill_rectangle(x, y, x + width - 1, y + width - 1, color, lcd_info);
		linebrush_s(x, y, width, color, lcd_info);

		d = 2 * abs(delta_y) - abs(delta_x);   /* 初始判别式 */

		/* 步进判断x,y坐标点 */
        for(i = min_x; i < max_x; i++)
        {
            x++;
            if(d >= 0)
            {
                if (delta_x * delta_y >= 0)
					y += 1;
				else
					y -= 1;                        //若d>=0,y(i+1)=y(i)±1
				d += 2 * (abs(delta_y) - abs(delta_x));    //更新d
            }
            else
            {
                d += 2 * abs(delta_y);
            }
			//linebrush_v(x, y, width, color, lcd_info);
			//fill_rectangle(x, y, x + width - 1, y + width - 1, color, lcd_info);
			linebrush_s(x, y, width, color, lcd_info);
		}
    }
    else   /* 斜率大于1 */
    {
        if (start_y < end_y) 
		{                     //步进方向为y轴，默认画点从下往上画          
			x = start_x;
			y = start_y;
            min_y = start_y;
            max_y = end_y;
		}
		else 
		{
			x = end_x;
			y = end_y;
            min_y = end_y;
            max_y = start_y;
		}

		//linebrush_h(x, y, width, color, lcd_info);
		//fill_rectangle(x, y, x + width - 1, y + width - 1, color, lcd_info);
		linebrush_s(x, y, width, color, lcd_info);
		d = 2 * abs(delta_x) - abs(delta_y); 

		for(i = min_y; i < max_y; i++)
        {
            y++;
            if(d >= 0)
            {
                if(delta_x * delta_y >= 0)
                    x += 1; 
                else
                    x -= 1;
                d += 2 * (abs(delta_x) - abs(delta_y));
            }
            else
            {
                d += 2 * abs(delta_x);
            }
			//linebrush_h(x, y, width, color, lcd_info);
			//fill_rectangle(x, y, x + width - 1, y + width - 1, color, lcd_info);
			linebrush_s(x, y, width, color, lcd_info);
		}
    }
}


/*************************************************************
绘制8分圆
***************************************************************/
static void draw_8_circle(int x, int y, int center_x, int center_y, unsigned int color, pLCD_INFO lcd_info)
{
	int put_x = center_x + lcd_info->xres/2;
	int put_y = center_y + lcd_info->yres/2;

	put_pixel(put_x + x, put_y - y, color, lcd_info);
	put_pixel(put_x + x, put_y + y, color, lcd_info);
	put_pixel(put_x + y, put_y + x, color, lcd_info);
	put_pixel(put_x - y, put_y + x, color, lcd_info);
	put_pixel(put_x - x, put_y + y, color, lcd_info);
	put_pixel(put_x - x, put_y - y, color, lcd_info);
	put_pixel(put_x - y, put_y - x, color, lcd_info);
	put_pixel(put_x + y, put_y - x, color, lcd_info);
}

/*************************************************************
画圆函数
***************************************************************/
static void draw_circel(int center_x, int center_y, unsigned short radius, unsigned short width, unsigned int color, pLCD_INFO lcd_info)
{
	/* Bresenham算法 */

	if(width < 1)  //默认宽度
		width = 1;
	
	int i = 0;
	do
	{
		int x = 0, y = radius + i;
		int d = 3 - 2 * radius + i;//判别式
				
		draw_8_circle(x, y, center_x, center_y, color, lcd_info);
		while(x < y)
		{
			if(d < 0)
			{
				d = d + 4 * x + 6;
			}
			else
			{
				d = d + 4 * ( x - y ) + 10;
		 	    y--;
			}
			x++;
			draw_8_circle(x, y, center_x, center_y, color, lcd_info);
		}
		i++;
	}
	while(width > i);
}

/************************************************************
画钟表刻度
***************************************************************/
static void draw_dial(unsigned short r, pLCD_INFO lcd_info)
{
	/* 根据弧度值绘制 */
	int x, y, i;
	for(i = 0; i < 60; i++)
	{
		y = (int)(r * sin(2 * PI * i / 60));
		x = (int)(r * cos(2 * PI * i / 60));

		if(i % 15 == 0) 
			fill_rectangle(x - 3, y - 3, x + 3, y + 3, RED, lcd_info);
		else if(i % 5 == 0)
			draw_circel(x , y, 2, 1, BLUE, lcd_info);
		else
			put_pixel(lcd_info->xres/2+x, lcd_info->yres/2-y, GREEN, lcd_info);
	}
}


/************************************************************
钟表文字
***************************************************************/
static void clock_text(unsigned short r, pLCD_INFO lcd_info)
{
	/* 根据弧度值写 */
	int x, y, i;
	for(i = 0; i < 60; i++)
	{
		switch(i)
		{
			case 0:
				y = (int)(r * sin(2 * PI * i / 60));
				x = (int)(r * cos(2 * PI * i / 60));
				show_ascii(x-1, y+6, '3', WHITE, lcd_info);
				break;
				
			case 5:
				y = (int)(r * sin(2 * PI * i / 60));
				x = (int)(r * cos(2 * PI * i / 60));
				show_ascii(x-1, y+8, '2', WHITE, lcd_info);
				break;

			case 10:
				y = (int)(r * sin(2 * PI * i / 60));
				x = (int)(r * cos(2 * PI * i / 60));
				show_ascii(x-2, y+8, '1', WHITE, lcd_info);
				break;

			case 15:
				y = (int)(r * sin(2 * PI * i / 60));
				x = (int)(r * cos(2 * PI * i / 60));
				show_text(x-8, y+8, 1, "12", WHITE, lcd_info);
				break;

			case 20:
				y = (int)(r * sin(2 * PI * i / 60));
				x = (int)(r * cos(2 * PI * i / 60));
				show_text(x-4, y+8, 1, "11", WHITE, lcd_info);
				break;

			case 25:
				y = (int)(r * sin(2 * PI * i / 60));
				x = (int)(r * cos(2 * PI * i / 60));
				show_text(x-4, y+8, 1, "10", WHITE, lcd_info);
				break;

			case 30:
				y = (int)(r * sin(2 * PI * i / 60));
				x = (int)(r * cos(2 * PI * i / 60));
				show_ascii(x-7, y+6, '9', WHITE, lcd_info);
				break;

			case 35:
				y = (int)(r * sin(2 * PI * i / 60));
				x = (int)(r * cos(2 * PI * i / 60));
				show_ascii(x-5, y+4, '8', WHITE, lcd_info);
				break;

			case 40:
				y = (int)(r * sin(2 * PI * i / 60));
				x = (int)(r * cos(2 * PI * i / 60));
				show_ascii(x-2, y+4, '7', WHITE, lcd_info);
				break;

			case 45:
				y = (int)(r * sin(2 * PI * i / 60));
				x = (int)(r * cos(2 * PI * i / 60));
				show_ascii(x-3, y+4, '6', WHITE, lcd_info);
				break;

			case 50:
				y = (int)(r * sin(2 * PI * i / 60));
				x = (int)(r * cos(2 * PI * i / 60));
				show_ascii(x-3, y+3, '5', WHITE, lcd_info);
				break;

			case 55:
				y = (int)(r * sin(2 * PI * i / 60));
				x = (int)(r * cos(2 * PI * i / 60));
				show_ascii(x-1, y+4, '4', WHITE, lcd_info);
				break;

			default:
				break;
		}
		
	}
}



/************************************************************
画钟表指针
***************************************************************/
static void draw_hand(int hour, int minute, int second, pLCD_INFO lcd_info)
{
	/* 时分秒的弧度 */
	double h_radian, m_radian, s_radian;
	/* 时分秒指针末端坐标 */
	int x_h, y_h, x_m, y_m, x_s, y_s;

	/* 计算弧度：
	 * 秒针一秒钟旋转：2π / 60 弧度
	 * 分针一秒钟旋转：2π / 60 / 60  弧度
	 * 时针一秒钟旋转：2π / 12 / 3600  弧度
	 */ 
	s_radian = (2 * PI * second / 60 + PI / 2);
	m_radian = (2 * PI * minute / 60 + 2 * PI * second / 3600 + PI / 2);
	h_radian = (2 * PI * hour / 12 + 2 * PI * (minute * 60  + second) / 3600 / 12 + PI / 2);

	/* 计算指针的末端坐标 */
	x_s = (int)(85 * cos(s_radian));
	y_s = (int)(85 * sin(s_radian));
	x_m = (int)(70 * cos(m_radian));
	y_m = (int)(70 * sin(m_radian));
	x_h = (int)(53 * cos(h_radian));	
	y_h = (int)(53 * sin(h_radian));
	
	/* 绘制指针，通过原点对称，确定起始点 */
	draw_line((int)(0.2 * x_h), (int)(0.2 * -y_h), -x_h, y_h, 4, RED, lcd_info);
	draw_line((int)(0.26 * x_m), (int)(0.26 * -y_m), -x_m, y_m, 3, BLUE, lcd_info);
	draw_line((int)(0.32 * x_s), (int)(0.32 * -y_s), -x_s, y_s, 2, GREEN, lcd_info);
	draw_circel(0, 0, 1, 5, WHITE, lcd_info);  //表盘中心

	sleep(1);

	/* 把表针设置成背景色，起到清除作用 */
	draw_line((int)(0.2 * x_h), (int)(0.2 * -y_h), -x_h, y_h, 4, BLACK, lcd_info);
	draw_line((int)(0.26 * x_m), (int)(0.26 * -y_m), -x_m, y_m, 3, BLACK, lcd_info);
	draw_line((int)(0.32 * x_s), (int)(0.32 * -y_s), -x_s, y_s, 2, BLACK, lcd_info);

}

/************************************************************
获取时间
***************************************************************/
static void get_time(pMYTIME t)
{
	time_t timeformin = time(NULL); //获取秒数时间 
	struct tm *gt;

	gt = localtime(&timeformin);   //格式化的时间

	int tmp = gt->tm_hour;
	if(tmp > 12)
		tmp -= 12;
	
	t->hour = tmp;
	t->min  = gt->tm_min;
	t->sec  = gt->tm_sec;
}



int main(int argc, char **argv)
{
	LCD_INFO tLcd_Info;
	MYTIME   timeforclock;
	lcd_init(&tLcd_Info);
	draw_circel(0, 0, 120, 2, GREEN, &tLcd_Info);
	draw_dial(113, &tLcd_Info);
	clock_text(100, &tLcd_Info);
	
	while(1)
	{
		get_time(&timeforclock);	
		draw_circel(0, 0, 35, 2, GREEN, &tLcd_Info);
		draw_hand(timeforclock.hour, timeforclock.min, timeforclock.sec, &tLcd_Info);
	}

	lcd_del(&tLcd_Info);
	return 0;
}


