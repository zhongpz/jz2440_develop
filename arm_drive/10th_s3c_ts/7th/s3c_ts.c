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

static struct input_dev *s3c_ts_dev;   //����һ��input_dev����
static struct clk *adc_clk;            //����һ��ʱ��
static struct timer_list ts_timer;     //����һ����ʱ��

/*������Ҫ�����ļĴ����ṹ��*/
struct s3c_ts_regs {
	unsigned long adccon;
	unsigned long adctsc;
	unsigned long adcdly;
	unsigned long adcdat0;
	unsigned long adcdat1;
	unsigned long adcupdn;
};

static volatile struct s3c_ts_regs *s3c_ts_regs;

/*����ȴ������ж�*/
static void enter_wait_pen_down_mode(void)
{
	//���˼Ĵ����� adctsc �Ĵ�������Ϊ 0xd3 �ͽ��롰�ȴ������ж�ģʽ�� ��
	s3c_ts_regs->adctsc = 0xd3;
}

/*����ȴ��ɿ��ж�*/
static void enter_wait_pen_up_mode(void)
{
	//���˼Ĵ����� adctsc �Ĵ�������Ϊ 0x1d3 �ͽ��롰�ȴ��ɿ��ж�ģʽ�� ��
	s3c_ts_regs->adctsc = 0x1d3;
}

/*����xy�Զ�����ģʽ*/
static void enter_measure_xy_mode(void)
{
	//����adctsc
	s3c_ts_regs->adctsc = (1<<3)|(1<<2);
}


/*����ADC����*/
static void start_adc(void)
{
	
	s3c_ts_regs->adccon |= (1<<0);//ADCCON ADC ���ƼĴ��� bit0 ���� 1 ��������
}

/*ADC������ʱ����*/
static void adc_delay(void)
{
	s3c_ts_regs->adcdly = 0xffff;  //��ʱʱ����Ϊ��󣬵ȵ�ѹ�ȶ�������adc
}

/*������˺���*/
static int s3c_filter_ts(int x[], int y[])
{
#define ERR_LIMIT 10

	int avr_x, avr_y;
	int det_x, det_y;

	avr_x = (x[0] + x[1])/2;
	avr_y = (y[0] + y[1])/2;

	det_x = (x[2] > avr_x) ? (x[2] - avr_x) : (avr_x - x[2]);
	det_y = (y[2] > avr_y) ? (y[2] - avr_y) : (avr_y - y[2]);

	if ((det_x > ERR_LIMIT) || (det_y > ERR_LIMIT))
		return 0;

	avr_x = (x[1] + x[2])/2;
	avr_y = (y[1] + y[2])/2;

	det_x = (x[3] > avr_x) ? (x[3] - avr_x) : (avr_x - x[3]);
	det_y = (y[3] > avr_y) ? (y[3] - avr_y) : (avr_y - y[3]);

	if ((det_x > ERR_LIMIT) || (det_y > ERR_LIMIT))
		return 0;
	
	return 1;
}

/*�����������жϴ�����*/
static irqreturn_t pen_down_up_irq(int irq, void *dev_id)
{
	if(s3c_ts_regs->adcdat0 & (1<<15))
	{
		/*1�ɿ�״̬*/
		//printk("pen up\n");
		input_report_abs(s3c_ts_dev, ABS_PRESSURE, 0);
		input_report_key(s3c_ts_dev, BTN_TOUCH, 0);
		input_sync(s3c_ts_dev);
		enter_wait_pen_down_mode();   //�ȴ�����
	}
	else
	{
		/*0����״̬*/	
		//printk("pen down\n");	
		//enter_wait_pen_up_mode();   //�ȴ��ɿ�
		
		/* ����֮�����xy����ģʽ */
		enter_measure_xy_mode();  
		
		/***********   �Ż���ʩһ:  ***************/
		/*ADC������ʱ���ȴ���ѹ�ȶ�������ADC*/
		adc_delay();
		
		/* ����ADC */
		start_adc();
	}

	return IRQ_HANDLED;
}




/*adcת�������жϴ�����*/
static irqreturn_t adc_irq(int irq, void *dev_id)
{
	/***********   �Ż���ʩ��:  ***************/
	/*��� ADC ���ʱ, ���ִ������Ѿ��ɿ�, �����˴ν�� */
	//static int cnt = 0;
	//static int adcdat0, adcdat1;

	//adcdat0 = s3c_ts_regs->adcdat0 & 0x3ff;
	//adcdat1 = s3c_ts_regs->adcdat1 & 0x3ff;

	//if(s3c_ts_regs->adcdat0 & (1<<15))
	//{
		/*1�ɿ�*/
	//	enter_wait_pen_down_mode();  //�ȴ����رʰ��£���ʱ����ӡ
	//}
	//else
	//{
		/*0����*/
	//	printk("adc_irq cnt = %d, x = %d, y = %d\n", ++cnt, adcdat0, adcdat1);
	//	enter_wait_pen_up_mode(); 
	//}

	/***********   �Ż���ʩ��:  ***************/
	         /*��β���ȡƽ��ֵ*/
	
	static int cnt = 0;
	static int adcdat0, adcdat1;
	static int x[4], y[4];
	
	adcdat0 = s3c_ts_regs->adcdat0 & 0x3ff;
	adcdat1 = s3c_ts_regs->adcdat1 & 0x3ff;

	if(s3c_ts_regs->adcdat0 & (1<<15))
	{
		/*1�ɿ�*/
		cnt = 0;
		input_report_abs(s3c_ts_dev, ABS_PRESSURE, 0);
		input_report_key(s3c_ts_dev, BTN_TOUCH, 0);
		input_sync(s3c_ts_dev);
		enter_wait_pen_down_mode();  //�ȴ����رʰ��£���ʱ����ӡ
	}
	else
	{
		/*0����*/
		x[cnt] = adcdat0;
		y[cnt] = adcdat1;
		cnt ++;
		if(cnt == 4)
		{
			if(s3c_filter_ts(x, y))
			{
				/*�����Ĵ�ȡƽ��ֵ*/
				//printk("adc_irq , x = %d, y = %d\n", (x[0]+x[1]+x[2]+x[3])/4, (y[0]+y[1]+y[2]+y[3])/4);
				/*�ϱ��¼�*/

				input_report_abs(s3c_ts_dev, ABS_X, (x[0]+x[1]+x[2]+x[3])/4);  //x����
				input_report_abs(s3c_ts_dev, ABS_Y, (y[0]+y[1]+y[2]+y[3])/4);  //y����
				input_report_abs(s3c_ts_dev, ABS_PRESSURE, 1);    //1������
				input_report_key(s3c_ts_dev, BTN_TOUCH, 1);       //1������
				input_sync(s3c_ts_dev);
			}
			cnt = 0;
			enter_wait_pen_up_mode(); 

			/*������ʱ��*/
			mod_timer(&ts_timer, jiffies + HZ/100);
		}
		else
		{
			/*û�дﵽ�ĴΣ����ٴν���xy����ģʽ*/
			enter_measure_xy_mode();
			start_adc();	
		}
	}

	return IRQ_HANDLED;
}

/*��ʱ���жϴ�����*/
static void s3c_ts_timer_funtion(unsigned long data)
{
	if (s3c_ts_regs->adcdat0 & (1<<15))
	{
		/* �Ѿ��ɿ� */
		input_report_abs(s3c_ts_dev, ABS_PRESSURE, 0);  
		input_report_key(s3c_ts_dev, BTN_TOUCH, 0);
		input_sync(s3c_ts_dev);
		enter_wait_pen_down_mode();
	}
	else
	{
		/* ����X/Y���� */
		enter_measure_xy_mode();
		start_adc();
	}
}



/*1.��ں���*/
static int s3c_ts_init(void)
{
	/*1.1����һ��input_dev�ṹ��*/
	s3c_ts_dev = input_allocate_device();
	
	/*1.2����input_dev�ṹ��*/
	/*1.2.1���ò��������¼�*/
	set_bit(EV_KEY, s3c_ts_dev->evbit);    //�������¼�
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
	/*1.4.1ʹ����ʱ�ӣ�����ϵͳADC����ģ�飬����CLKCON[15]Ϊ1*/
	adc_clk = clk_get(NULL, "adc");
	clk_enable(adc_clk);

	/*1.4.2ioremap�Ĵ���*/
	s3c_ts_regs = ioremap(0x58000000, sizeof(s3c_ts_regs));

	/*1.4.3���üĴ���*/

	/*  adccon����
	 *	PRSCEN [14]��Ԥ��Ƶʹ�ܡ�Ҫ����Ϊ��1��
	 *  PRSCVL [13:6]��Ԥ��Ƶϵ��������Ϊ49������ ADC ʱ�ӣ�
	 *                 ADCCLK=PCLK/(49+1)=50MHz/(49+1)=1MHz
	 *	STDBM [2]:����ģʽѡ������Ϊ0����������ģʽ
	 *  READ_ START [1]������Ϊ0,��ʹ�ö�����������ADת����
	 */
	 s3c_ts_regs->adccon = (1<<14) | (49<<6);

	
	/*1.4.4ע���ж�*/
	 
	 request_irq(IRQ_TC, pen_down_up_irq, IRQF_SAMPLE_RANDOM, "ts_pen", NULL); //�������ж�
	 request_irq(IRQ_ADC, adc_irq, IRQF_SAMPLE_RANDOM, "adc", NULL);   //ADC�����ж�

	 enter_wait_pen_down_mode();   //����ȴ������ж�

	/*1.4.5��ʼ����ʱ��*/
	init_timer(&ts_timer);
	ts_timer.function = s3c_ts_timer_funtion;
	add_timer(&ts_timer);
	 
	return 0;
}



/*2.���ں���*/
static void s3c_ts_exit(void)
{
	input_unregister_device(s3c_ts_dev);  /*ж��input_dev�ṹ*/
	input_free_device(s3c_ts_dev);  
	free_irq(IRQ_TC, NULL);             //�ͷ��ж�
	free_irq(IRQ_ADC, NULL);            //�ͷ��ж�
	iounmap(s3c_ts_regs);      
	del_timer(&ts_timer);
}

/*3.����*/
module_init(s3c_ts_init);
module_exit(s3c_ts_exit);

MODULE_LICENSE("GPL");

