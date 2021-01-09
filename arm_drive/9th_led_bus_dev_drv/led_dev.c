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



/*定义资源*/
static struct resource led_resource[] = {
    [0] = {
        .start = 0x56000050,         //起始物理地址         
        .end   = 0x56000050 + 8 - 1,  //结束物理地址
        .flags = IORESOURCE_MEM,        //类型:内存
    },
    [1] = {
        .start = 4,  //第一个LED的引脚为4，要换第二个LED就换成5
        .end   = 4,
        .flags = IORESOURCE_IRQ,     //类型:中断
    },
    [2] = {
        .start = 5,  //第1个中断类型资源，上一个为第0个。
        .end   = 5,
        .flags = IORESOURCE_IRQ,     //类型:中断
    },
};

/*定义release函数*/
void led_release(struct device *dev)
{

}


/*1.定义平台设备platform_device结构体*/
static struct platform_device led_dev = {
    .name           = "myled",                      //名字
    .id             = -1,
    .num_resources  = ARRAY_SIZE(led_resource),    //资源大小
    .resource       = led_resource,                //资源
    .dev            ={
		.release    = led_release,
		},
};

/*2.入口行数*/
static int led_dev_init(void)
{
	/*2.1注册一个平台设备*/
	platform_device_register(&led_dev);
	return 0;
}

/*3.出口函数*/
static void led_dev_exit(void)
{
	platform_device_unregister(&led_dev);
}


/*4.修饰*/
module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");








