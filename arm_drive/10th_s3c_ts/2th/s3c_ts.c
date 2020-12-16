#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/clk.h>	
#include <linux/gpio_keys.h>
#include <asm/irq.h>
#include <asm/plat-s3c24xx/ts.h>
#include <asm/arch/regs-adc.h>
#include <asm/gpio.h>

#include <asm/io.h>
#include <asm/arch/regs-gpio.h>


static struct input_dev *s3c_ts_dev;   //定义一个input_dev变量
static struct clk *adc_clk;            //定义一个时钟

/*定义需要操作的寄存器结构体*/
struct s3c_ts_regs {
	unsigned long adccon;
	unsigned long adctsc;
	unsigned long adcdly;
	unsigned long adcdat0;
	unsigned long adcdat1;
	unsigned long adcupdn;
};

static volatile struct s3c_ts_regs *s3c_ts_regs;

/*等待进入按下中断函数*/
static void enter_wait_pen_down_mode(void)
{
	//将此寄存器的 adctsc 寄存器设置为 0xd3 就进入“等待按下中断模式” 。
	s3c_ts_regs->adctsc = 0xd3;
}

/*等待进入松开中断函数*/
static void enter_wait_pen_up_mode(void)
{
	//将此寄存器的 adctsc 寄存器设置为 0x1d3 就进入“等待松开中断模式” 。
	s3c_ts_regs->adctsc = 0x1d3;
}

/*中断处理函数*/
static irqreturn_t pen_down_up_irq(int irq, void *dev_id)
{
	if(s3c_ts_regs->adcdat0 & (1<<15))
	{
		/*1松开状态*/
		printk("pen up\n");
		enter_wait_pen_down_mode();   //等待按下
	}
	else
	{
		/*0按下状态*/	
		printk("pen down\n");
		enter_wait_pen_up_mode();   //等待松开
	}

	return IRQ_HANDLED;
}
/*1.入口函数*/
static int s3c_ts_init(void)
{
	/*1.1分配一个input_dev结构体*/
	s3c_ts_dev = input_allocate_device();
	
	/*1.2设置input_dev结构体*/
	/*1.2.1设置产生哪类事件*/
	set_bit(EV_KEY, s3c_ts_dev->evbit);    //按键类事件
	set_bit(EV_ABS, s3c_ts_dev->evbit);   //触摸屏绝对位移事件
	
	/*1.2.2*产生这类事件中的哪种事件*/
	set_bit(BTN_TOUCH, s3c_ts_dev->keybit);    //触摸屏事件

	/*1.2.3触摸屏事件参数设置*/
	input_set_abs_params(s3c_ts_dev, ABS_X, 0, 0x3FF, 0, 0);     //x方向参数，因为是10位ADC，所以最大为0x3FF
	input_set_abs_params(s3c_ts_dev, ABS_Y, 0, 0x3FF, 0, 0);     //y方向参数
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 1, 0, 0);  //按压参数

	/*1.3注册input_dev结构体*/
	input_register_device(s3c_ts_dev);
	
	
	/*1.4硬件相关的操作*/
	/*1.4.1使用能时钟，开启系统ADC功能模块，设置CLKCON[15]为1*/
	adc_clk = clk_get(NULL, "adc");
	clk_enable(adc_clk);

	/*1.4.2ioremap寄存器*/
	s3c_ts_regs = ioremap(0x58000000, sizeof(s3c_ts_regs));

	/*1.4.3设置寄存器*/

	/*  adccon设置
	 *	PRSCEN [14]：预分频使能。要设置为“1”
	 *  PRSCVL [13:6]：预分频系数。设置为49，所以 ADC 时钟：
	 *                 ADCCLK=PCLK/(49+1)=50MHz/(49+1)=1MHz
	 *	STDBM [2]:工作模式选择。设置为0，正常工作模式
	 *  READ_ START [1]：设置为0,不使用读操作来启动AD转换。
	 */
	 s3c_ts_regs->adccon |= (1<<14) | (49<<6) | (0<<2);

	
	/*注册触摸屏中断*/
	 enter_wait_pen_down_mode();   //进入等待按下中断
	 request_irq(IRQ_TC, pen_down_up_irq, IRQF_SAMPLE_RANDOM, "ts_pen", NULL);
	 
	return 0;
}



/*2.出口函数*/
static void s3c_ts_exit(void)
{
	input_unregister_device(s3c_ts_dev);  /*卸载input_dev结构*/
	input_free_device(s3c_ts_dev);  
	free_irq(IRQ_TC, NULL);             //释放中断
	iounmap(s3c_ts_regs);      
}

/*3.修饰*/
module_init(s3c_ts_init);
module_exit(s3c_ts_exit);

MODULE_LICENSE("GPL");
