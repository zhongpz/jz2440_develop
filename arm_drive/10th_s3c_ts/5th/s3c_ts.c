#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <asm/plat-s3c24xx/ts.h>

#include <asm/arch/regs-adc.h>
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

/*进入等待按下中断*/
static void enter_wait_pen_down_mode(void)
{
	//将此寄存器的 adctsc 寄存器设置为 0xd3 就进入“等待按下中断模式” 。
	s3c_ts_regs->adctsc = 0xd3;
}

/*进入等待松开中断*/
static void enter_wait_pen_up_mode(void)
{
	//将此寄存器的 adctsc 寄存器设置为 0x1d3 就进入“等待松开中断模式” 。
	s3c_ts_regs->adctsc = 0x1d3;
}

/*进入xy自动测量模式*/
static void enter_measure_xy_mode(void)
{
	//设置adctsc
	s3c_ts_regs->adctsc = (1<<3)|(1<<2);
}


/*启动ADC函数*/
static void start_adc(void)
{
	
	s3c_ts_regs->adccon |= (1<<0);//ADCCON ADC 控制寄存器 bit0 设置 1 即可启动
}

/*ADC启动延时函数*/
static void adc_delay(void)
{
	s3c_ts_regs->adcdly = 0xffff;  //延时时间设为最大，等电压稳定后启动adc
}


/*触摸屏按下中断处理函数*/
static irqreturn_t pen_down_up_irq(int irq, void *dev_id)
{
	if(s3c_ts_regs->adcdat0 & (1<<15))
	{
		/*1松开状态*/
		//printk("pen up\n");
		enter_wait_pen_down_mode();   //等待按下
	}
	else
	{
		/*0按下状态*/	
		//printk("pen down\n");	
		//enter_wait_pen_up_mode();   //等待松开
		
		/* 按下之后进入xy测量模式 */
		enter_measure_xy_mode();  
		
		/***********   优化措施一:  ***************/
		/*ADC启动延时，等待电压稳定后启动ADC*/
		adc_delay();
		
		/* 启动ADC */
		start_adc();
	}

	return IRQ_HANDLED;
}


/*adc转换结束中断处理函数*/
static irqreturn_t adc_irq(int irq, void *dev_id)
{
	/***********   优化措施二:  ***************/
	/*如果 ADC 完成时, 发现触摸笔已经松开, 则丢弃此次结果 */
	static int cnt = 0;
	static int adcdat0, adcdat1;

	adcdat0 = s3c_ts_regs->adcdat0 & 0x3ff;
	adcdat1 = s3c_ts_regs->adcdat1 & 0x3ff;

	if(s3c_ts_regs->adcdat0 & (1<<15))
	{
		/*1松开*/
		enter_wait_pen_down_mode();  //等待触控笔按下，此时不打印
	}
	else
	{
		/*0按下*/
		printk("adc_irq cnt = %d, x = %d, y = %d\n", ++cnt, adcdat0, adcdat1);
		enter_wait_pen_up_mode(); 
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
	 s3c_ts_regs->adccon = (1<<14) | (49<<6);

	
	/*注册中断*/
	 
	 request_irq(IRQ_TC, pen_down_up_irq, IRQF_SAMPLE_RANDOM, "ts_pen", NULL); //触摸屏中断
	 request_irq(IRQ_ADC, adc_irq, IRQF_SAMPLE_RANDOM, "adc", NULL);   //ADC结束中断

	 enter_wait_pen_down_mode();   //进入等待按下中断
	 
	return 0;
}



/*2.出口函数*/
static void s3c_ts_exit(void)
{
	input_unregister_device(s3c_ts_dev);  /*卸载input_dev结构*/
	input_free_device(s3c_ts_dev);  
	free_irq(IRQ_TC, NULL);             //释放中断
	free_irq(IRQ_ADC, NULL);            //释放中断
	iounmap(s3c_ts_regs);      
}

/*3.修饰*/
module_init(s3c_ts_init);
module_exit(s3c_ts_exit);

MODULE_LICENSE("GPL");

