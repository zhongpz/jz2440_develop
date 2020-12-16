
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


static struct fb_info *s3c_lcd_fb;   /*����һ��fb_info�ṹ��*/

/*Ҫӳ���gpio����*/
static volatile unsigned long *gpbcon;
static volatile unsigned long *gpbdat;  /*gpb0���ڱ���ʹ�ܣ�0�أ�1��*/
static volatile unsigned long *gpccon;
static volatile unsigned long *gpdcon;
static volatile unsigned long *gpgcon;

/*lcd���ƼĴ���*/
struct lcd_regs {
	unsigned long	lcdcon1;
	unsigned long	lcdcon2;
	unsigned long	lcdcon3;
	unsigned long	lcdcon4;
	unsigned long	lcdcon5;
	unsigned long	lcdsaddr1;
	unsigned long	lcdsaddr2;
	unsigned long	lcdsaddr3;
	unsigned long	redlut;
	unsigned long	greenlut;
	unsigned long	bluelut; 
	unsigned long	reserved[9];  /*��ַ��Ծ*/
	unsigned long	dithmode;  
	unsigned long	tpal; 
	unsigned long	lcdintpnd; 
	unsigned long	lcdsrcpnd;  
	unsigned long	lcdintmsk;  
	unsigned long	lpcsel;
};


static volatile struct lcd_regs *lcd_regs;

/*���ڲ�����ɫ�ĺ���������д�õ�*/
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
	gpbcon = ioremap(0x56000010, 8);
	gpbdat = gpbcon + 1;

	gpccon = ioremap(0x56000020, 4);
	gpdcon = ioremap(0x56000030, 4);
	gpgcon = ioremap(0x56000060, 4);

	*gpccon  = 0xaaaaaaaa;   /* GPIO�ܽ�����VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND */
	*gpdcon  = 0xaaaaaaaa;   /* GPIO�ܽ�����VD[23:8] */
	
	*gpbcon &= ~(3);   /* GPB0����Ϊ������� */
	*gpbcon |= 1;
	*gpbdat &= ~1;     /* ����͵�ƽ */

	*gpgcon |= (3<<8); /* GPG4����LCD_PWREN */
	

	/* 1.3.2����lcd�ֲ�����lcd������������vclk�� */
	lcd_regs = ioremap(0x4d000000, sizeof(struct lcd_regs));

	/* bit[17:8]: VCLK = HCLK / [(CLKVAL+1) x 2], LCD�ֲ�P14
	 *            10MHz(100ns) = 100MHz / [(CLKVAL+1) x 2]
	 *            CLKVAL = 4
	 * bit[6:5]: 0b11, TFT LCD
	 * bit[4:1]: 0b1100, 16 bpp for TFT
	 * bit[0]  : 0 = Disable the video output and the LCD control signal.
	 */
	lcd_regs->lcdcon1  = (4<<8) | (3<<5) | (0xc<<1);

	/* ��ֱ�����ʱ�����
	 * bit[31:24]: VBPD, VSYNC֮���ٹ��೤ʱ����ܷ�����1������
	 *             LCD�ֲ� T0-T2-T1=4
	 *             VBPD=3
	 * bit[23:14]: ������, 320, ����LINEVAL=320-1=319
	 * bit[13:6] : VFPD, �������һ������֮���ٹ��೤ʱ��ŷ���VSYNC
	 *             LCD�ֲ�T2-T5=322-320=2, ����VFPD=2-1=1
	 * bit[5:0]  : VSPW, VSYNC�źŵ�������, LCD�ֲ�T1=1, ����VSPW=1-1=0
	 */
	lcd_regs->lcdcon2  = (3<<24) | (319<<14) | (1<<6) | (0<<0);

	/* ˮƽ�����ʱ�����
	 * bit[25:19]: HBPD, VSYNC֮���ٹ��೤ʱ����ܷ�����1������
	 *             LCD�ֲ� T6-T7-T8=17
	 *             HBPD=16
	 * bit[18:8]: ������, 240, ����HOZVAL=240-1=239
	 * bit[7:0] : HFPD, �������һ�������һ����������֮���ٹ��೤ʱ��ŷ���HSYNC
	 *             LCD�ֲ�T8-T11=251-240=11, ����HFPD=11-1=10
	 */
	lcd_regs->lcdcon3 = (16<<19) | (239<<8) | (10<<0);

	/* ˮƽ�����ͬ���ź�
	 * bit[7:0]	: HSPW, HSYNC�źŵ�������, LCD�ֲ�T7=5, ����HSPW=5-1=4
	 */	
	lcd_regs->lcdcon4 = 4;

	/* �źŵļ��� 
	 * bit[11]: 1=565 format
	 * bit[10]: 0 = The video data is fetched at VCLK falling edge
	 * bit[9] : 1 = HSYNC�ź�Ҫ��ת,���͵�ƽ��Ч 
	 * bit[8] : 1 = VSYNC�ź�Ҫ��ת,���͵�ƽ��Ч 
	 * bit[6] : 0 = VDEN���÷�ת
	 * bit[3] : 0 = PWREN���0
	 * bit[1] : 0 = BSWP
	 * bit[0] : 1 = HWSWP 2440�ֲ�P413
	 */
	lcd_regs->lcdcon5 = (1<<11) | (0<<10) | (1<<9) | (1<<8) | (1<<0);

	

	/* 1.3.3�����Դ�(framebuffer)�����ѵ�ַ����lcd������ */
	//s3c_lcd_fb->fix.smem_start = ...   /*�Դ��������ʼ��ַ*/
	s3c_lcd_fb->screen_base = dma_alloc_writecombine(NULL, s3c_lcd_fb->fix.smem_len, &s3c_lcd_fb->fix.smem_start, GFP_KERNEL);
	
	lcd_regs->lcdsaddr1  = (s3c_lcd_fb->fix.smem_start >> 1) & ~(3<<30);
	lcd_regs->lcdsaddr2  = ((s3c_lcd_fb->fix.smem_start + s3c_lcd_fb->fix.smem_len) >> 1) & 0x1fffff;
	lcd_regs->lcdsaddr3  = (240*16/16);  /* һ�еĳ���(��λ: 2�ֽ�) */	
	
	//s3c_lcd->fix.smem_start = xxx;  /* �Դ�������ַ */
	/* ����LCD */
	lcd_regs->lcdcon1 |= (1<<0); /* ʹ��LCD���� */
	lcd_regs->lcdcon5 |= (1<<3); /* ʹ��LCD���� */
	*gpbdat |= 1;     /* ����ߵ�ƽ, ʹ�ܱ��� */

	
	/***************** 1.4ע��fb_info�ṹ�� *******************/
	register_framebuffer(s3c_lcd_fb);
	return 0;
}


/* 2.���ں��� */
static void s3c_lcd_exit(void)
{
	unregister_framebuffer(s3c_lcd_fb);
	lcd_regs->lcdcon1 &= ~(1<<0); /* �ر�LCD���� */
	*gpbdat &= ~1;     /* �رձ��� */
	dma_free_writecombine(NULL, s3c_lcd_fb->fix.smem_len, s3c_lcd_fb->screen_base, s3c_lcd_fb->fix.smem_start);
	iounmap(lcd_regs);
	iounmap(gpbcon);
	iounmap(gpccon);
	iounmap(gpdcon);
	iounmap(gpgcon);
	framebuffer_release(s3c_lcd);
	
}


/* 3.���� */
module_init(s3c_lcd_init);
module_exit(s3c_lcd_exit);

MODULE_LICENSE("GPL");





