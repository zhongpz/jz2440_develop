
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <asm/mach/map.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/fb.h>


static struct fb_info *s3c_lcd_fb;   //����һ��fb_info�ṹ��

static struct fb_ops s3c_lcdfb_ops = {
	.owner		= THIS_MODULE,
//	.fb_setcolreg	= atmel_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

/* 1.��ں���  */
static int s3c_lcd_init(void)
{
	/***************** 1.1����һ��fb_info�ṹ�� ******************/
	if(!(s3c_lcd_fb = framebuffer_alloc(0, NULL)))
	{
		printk("framebuffer_alloc error!\n");
		return -1;
	}

	/***************** 1.2����fb_info�ṹ�� ********************/
	/* 1.2.1�̶��������� */
	strcpy(s3c_lcd_fb->fix.id, "mylcd");     /*���֣����ȡ*/
	s3c_lcd_fb->fix.smem_len    = 240*320*16;   /*�Դ泤��*/
	s3c_lcd_fb->fix.type        = FB_TYPE_PACKED_PIXELS;  /*���ͣ�ʹ��Ĭ��*/
	s3c_lcd_fb->fix.visual      = FB_VISUAL_TRUECOLOR;    /*TFT���ɫ*/
	s3c_lcd_fb->fix.line_length = 240*2;       /*�г���(һ��240������)����λ���ֽ�*/
	
	/* 1.2.2�ɱ��������� */
	s3c_lcd_fb->var.xres = 240;    /*x����ֱ���*/
	s3c_lcd_fb->var.xres = 320;    /*y����ֱ���*/
	s3c_lcd_fb->var.xres_virtual   = 240;  /*x����ķֱ���*/ 
	s3c_lcd_fb->var.yres_virtual   = 320;  /*y����ķֱ���*/
	s3c_lcd_fb->var.bits_per_pixel = 16;   /*ÿ�����ض���λ*/

	/*RGB: 565*/
	s3c_lcd_fb->var.red.offset   = 11;
	s3c_lcd_fb->var.red.length   = 5;

	s3c_lcd_fb->var.green.offset = 5;
	s3c_lcd_fb->var.green.length = 6;

	s3c_lcd_fb->var.blue.offset  = 0;
	s3c_lcd_fb->var.blue.length  = 5;

	s3c_lcd_fb->var.activate    = FB_ACTIVATE_NOW;
	
	/* 1.2.3���ò������� */
	s3c_lcd_fb->fbops = &s3c_lcdfb_ops;

	/* 1.2.4�������� */
	//s3c_lcd_fb->pseudo_palette =;            /*��ɫ��*/
	//s3c_lcd_fb->screen_base  = ;             /* �Դ�������ַ */ 
	s3c_lcd_fb->screen_size   = 240*324*16/8;  /*�Դ��С*/

	/***************** 1.3Ӳ����صĲ��� **********************/
	/* 1.3.1����gpio����lcd */



	

	/* 1.3.2����lcd�ֲ�����lcd������������vclk�� */

	/* 1.3.3�����Դ�(framebuffer)�����ѵ�ַ����lcd������ */
	//s3c_lcd_fb->fix.smem_start = ...   /*�Դ��������ʼ��ַ*/
	
	/***************** 1.4ע��fb_info�ṹ�� *******************/
	register_framebuffer(s3c_lcd_fb);
	return 0;
}


/* 2.���ں��� */
static void s3c_lcd_exit(void)
{
	unregister_framebuffer(s3c_lcd_fb);
}


/* 3.���� */
module_init(s3c_lcd_init);
module_exit(s3c_lcd_exit);

MODULE_LICENSE("GPL");





