
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
	strcmp(s3c_lcd_fb->fix.id, "mylcd");   /*���֣����ȡ*/
	

	
	/* 1.2.2�ɱ��������� */
	
	/* 1.2.3���ò������� */

	/* 1.2.4�������� */
	

	/***************** 1.3Ӳ����صĲ��� **********************/
	/* 1.3.1����gpio����lcd */

	/* 1.3.2����lcd�ֲ�����lcd������������vclk�� */

	/* 1.3.3�����Դ�(framebuffer)�����ѵ�ַ����lcd������ */

	
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





