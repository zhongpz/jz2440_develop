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


/*1.入口函数*/
static int s3c_ts_init(void)
{
	/*1.1分配一个input_dev结构体*/
	s3c_ts_dev = input_allocate_device();
	
	/*1.2设置input_dev结构体*/
	/*1.2.1设置产生哪类事件*/
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
	return 0;
}



/*2.出口函数*/
static void s3c_ts_exit(void)
{
	/*卸载input_dev结构*/
	input_unregister_device(s3c_ts_dev);
}



/*3.修饰*/
module_init(s3c_ts_init);
module_exit(s3c_ts_exit);

MODULE_LICENSE("GPL");