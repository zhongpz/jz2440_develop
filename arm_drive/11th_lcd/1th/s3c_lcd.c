
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
	strcmp(s3c_lcd_fb->fix.id, "mylcd");   /*名字，随便取*/
	

	
	/* 1.2.2可被参数设置 */
	
	/* 1.2.3设置操作函数 */

	/* 1.2.4其他设置 */
	

	/***************** 1.3硬件相关的操作 **********************/
	/* 1.3.1配置gpio用于lcd */

	/* 1.3.2根据lcd手册设置lcd控制器，例如vclk等 */

	/* 1.3.3分配显存(framebuffer)，并把地址告诉lcd控制器 */

	
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





