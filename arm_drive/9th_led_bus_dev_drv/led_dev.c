#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/mach-types.h>



/*������Դ*/
static struct resource led_resource[] = {
    [0] = {
        .start = 0x56000050,         //��ʼ�����ַ         
        .end   = 0x56000050 + 8 - 1,  //���������ַ
        .flags = IORESOURCE_MEM,        //����:�ڴ�
    },
    [1] = {
        .start = 4,  //��һ��LED������Ϊ4��Ҫ���ڶ���LED�ͻ���5
        .end   = 4,
        .flags = IORESOURCE_IRQ,     //����:�ж�
    },
    [2] = {
        .start = 5,  //��1���ж�������Դ����һ��Ϊ��0����
        .end   = 5,
        .flags = IORESOURCE_IRQ,     //����:�ж�
    },
};

/*����release����*/
void led_release(struct device *dev)
{

}


/*1.����ƽ̨�豸platform_device�ṹ��*/
static struct platform_device led_dev = {
    .name           = "myled",                      //����
    .id             = -1,
    .num_resources  = ARRAY_SIZE(led_resource),    //��Դ��С
    .resource       = led_resource,                //��Դ
    .dev            ={
		.release    = led_release,
		},
};

/*2.�������*/
static int led_dev_init(void)
{
	/*2.1ע��һ��ƽ̨�豸*/
	platform_device_register(&led_dev);
	return 0;
}

/*3.���ں���*/
static void led_dev_exit(void)
{
	platform_device_unregister(&led_dev);
}


/*4.����*/
module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");







