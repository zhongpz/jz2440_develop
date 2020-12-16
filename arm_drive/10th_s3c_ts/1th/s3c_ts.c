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


static struct input_dev *s3c_ts_dev;   //����һ��input_dev����


/*1.��ں���*/
static int s3c_ts_init(void)
{
	/*1.1����һ��input_dev�ṹ��*/
	s3c_ts_dev = input_allocate_device();
	
	/*1.2����input_dev�ṹ��*/
	/*1.2.1���ò��������¼�*/
	set_bit(EV_ABS, s3c_ts_dev->evbit);   //����������λ���¼�
	
	/*1.2.2*���������¼��е������¼�*/
	set_bit(BTN_TOUCH, s3c_ts_dev->keybit);    //�������¼�

	/*1.2.3�������¼���������*/
	input_set_abs_params(s3c_ts_dev, ABS_X, 0, 0x3FF, 0, 0);     //x�����������Ϊ��10λADC���������Ϊ0x3FF
	input_set_abs_params(s3c_ts_dev, ABS_Y, 0, 0x3FF, 0, 0);     //y�������
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 1, 0, 0);  //��ѹ����

	/*1.3ע��input_dev�ṹ��*/
	input_register_device(s3c_ts_dev);
	
	
	/*1.4Ӳ����صĲ���*/
	return 0;
}



/*2.���ں���*/
static void s3c_ts_exit(void)
{
	/*ж��input_dev�ṹ*/
	input_unregister_device(s3c_ts_dev);
}



/*3.����*/
module_init(s3c_ts_init);
module_exit(s3c_ts_exit);

MODULE_LICENSE("GPL");