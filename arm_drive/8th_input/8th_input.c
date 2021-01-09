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
#include <linux/gpio_keys.h>

#include <asm/gpio.h>

#include <asm/io.h>
#include <asm/arch/regs-gpio.h>

/*引脚描述结构体*/
struct pin_desc{
		int irq;                //中断号
		char *name;             //名字
		unsigned int pin;       //引脚
		unsigned int key_val;   //按键值
};

struct pin_desc pins_desc[4] = {
	{IRQ_EINT0,  "S2", S3C2410_GPF0,  KEY_L},
	{IRQ_EINT2,  "S3", S3C2410_GPF2,  KEY_S},
	{IRQ_EINT11, "S4", S3C2410_GPG3,  KEY_ENTER},
	{IRQ_EINT19, "S5", S3C2410_GPG11, KEY_LEFTSHIFT},
};

static struct pin_desc *irq_pd;          
static struct timer_list buttons_timer;  //定义定时器
static struct input_dev *buttons_dev;    //定义input_dev结构体

/* 中断处理函数 */
static irqreturn_t button_irq(int irq, void *dev_id)
{
	irq_pd = (struct pin_desc *)dev_id;
	/* 设定定时器超时时间为10ms */
	mod_timer(&buttons_timer, jiffies+HZ/100);
	return IRQ_RETVAL(IRQ_HANDLED);
}

/* 定时器处理函数 */
static void buttons_timer_funtion(unsigned long data)
{
	struct pin_desc *pindesc = irq_pd;
	unsigned int pinval;
	if(!pindesc)
		return;

	pinval = s3c2410_gpio_getpin(pindesc->pin);

	if(pinval)
	{
		/* 松开，上报事件， input_dev结构,事件类型，具体事件，0表示松开*/
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 0);	
		input_sync(buttons_dev); //上报同步事件
	}
	else
	{
		/*按下*/
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 1);	
		input_sync(buttons_dev); //上报同步事件
	}
}
	

/* 1.入口函数 */
static int buttons_init(void)
{
	/* 1.1分配一个input_dev结构体，正常要判断是否成功 */
	buttons_dev = input_allocate_device();		 
	
	/* 1.2设置结构体 */
	/* 1.2.1 能产生哪类事件 */
	set_bit(EV_KEY, buttons_dev->evbit);   //按键类事件
	set_bit(EV_REP, buttons_dev->evbit);   //重复类事件

	/* 1.2.2 能产生按键类事件中的哪种事件，用四个按键来产生L,S,ENTER,LEFTSHIFT */
	set_bit(KEY_L, buttons_dev->keybit); 
	set_bit(KEY_S, buttons_dev->keybit); 
	set_bit(KEY_ENTER, buttons_dev->keybit); 
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit); 
	
	/* 1.3 注册input_dev */
	input_register_device(buttons_dev);

	/* 1.4 硬件相关操作 */	
	/* 1.4.1 注册中断*/
	int i;
	for(i = 0; i < 4; i++)
	{
		request_irq(pins_desc[i].irq, button_irq, IRQT_BOTHEDGE, pins_desc[i].name, &pins_desc[i]);
	}

	/* 1.4.2 使用定时器 */

	/* 1.4.2.1 初始化定时器*/
	init_timer(&buttons_timer);

	/* 1.4.2.2定时器处理函数 */
	buttons_timer.function = buttons_timer_funtion;
	
	/* 1.4.2.3添加定时器 */
	add_timer(&buttons_timer);

	
	return 0;
}

/* 2. 出口函数 */
static void buttons_exit(void)
{
	int i ;
	/* 清中断 */
	for(i = 0; i < 4; i++)
	{
		free_irq(pins_desc[i].irq, &pins_desc[i]);
	}

	/* 清除定时器 */
	del_timer(&buttons_timer);

	/* 卸载input_dev结构体 */
	input_unregister_device(buttons_dev);

	/* 释放input_dev 空间 */
	input_free_device(buttons_dev);	
}

/* 3.修饰入口函数和出口函数 */
module_init(buttons_init);
module_exit(buttons_exit);

MODULE_LICENSE("GPL");





