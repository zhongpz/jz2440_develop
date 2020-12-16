
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


static struct fb_info *s3c_lcd_fb;   //定义一个fb_info结构体

static struct fb_ops s3c_lcdfb_ops = {
	.owner		= THIS_MODULE,
//	.fb_setcolreg	= atmel_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

/* 1.入口函数  */
static int s3c_lcd_init(void)
{
	/***************** 1.1分配一个fb_info结构体 ******************/
	if(!(s3c_lcd_fb = framebuffer_alloc(0, NULL)))
	{
		printk("framebuffer_alloc error!\n");
		return -1;
	}

	/***************** 1.2设置fb_info结构体 ********************/
	/* 1.2.1固定参数设置 */
	strcpy(s3c_lcd_fb->fix.id, "mylcd");     /*名字，随便取*/
	s3c_lcd_fb->fix.smem_len    = 240*320*16;   /*显存长度*/
	s3c_lcd_fb->fix.type        = FB_TYPE_PACKED_PIXELS;  /*类型，使用默认*/
	s3c_lcd_fb->fix.visual      = FB_VISUAL_TRUECOLOR;    /*TFT真彩色*/
	s3c_lcd_fb->fix.line_length = 240*2;       /*行长度(一行240个像素)，单位是字节*/
	
	/* 1.2.2可被参数设置 */
	s3c_lcd_fb->var.xres = 240;    /*x方向分辨率*/
	s3c_lcd_fb->var.xres = 320;    /*y方向分辨率*/
	s3c_lcd_fb->var.xres_virtual   = 240;  /*x虚拟的分辨率*/ 
	s3c_lcd_fb->var.yres_virtual   = 320;  /*y虚拟的分辨率*/
	s3c_lcd_fb->var.bits_per_pixel = 16;   /*每个像素多少位*/

	/*RGB: 565*/
	s3c_lcd_fb->var.red.offset   = 11;
	s3c_lcd_fb->var.red.length   = 5;

	s3c_lcd_fb->var.green.offset = 5;
	s3c_lcd_fb->var.green.length = 6;

	s3c_lcd_fb->var.blue.offset  = 0;
	s3c_lcd_fb->var.blue.length  = 5;

	s3c_lcd_fb->var.activate    = FB_ACTIVATE_NOW;
	
	/* 1.2.3设置操作函数 */
	s3c_lcd_fb->fbops = &s3c_lcdfb_ops;

	/* 1.2.4其他设置 */
	//s3c_lcd_fb->pseudo_palette =;            /*调色板*/
	//s3c_lcd_fb->screen_base  = ;             /* 显存的虚拟地址 */ 
	s3c_lcd_fb->screen_size   = 240*324*16/8;  /*显存大小*/

	/***************** 1.3硬件相关的操作 **********************/
	/* 1.3.1配置gpio用于lcd */



	

	/* 1.3.2根据lcd手册设置lcd控制器，例如vclk等 */

	/* 1.3.3分配显存(framebuffer)，并把地址告诉lcd控制器 */
	//s3c_lcd_fb->fix.smem_start = ...   /*显存的物理起始地址*/
	
	/***************** 1.4注册fb_info结构体 *******************/
	register_framebuffer(s3c_lcd_fb);
	return 0;
}


/* 2.出口函数 */
static void s3c_lcd_exit(void)
{
	unregister_framebuffer(s3c_lcd_fb);
}


/* 3.修饰 */
module_init(s3c_lcd_init);
module_exit(s3c_lcd_exit);

MODULE_LICENSE("GPL");





