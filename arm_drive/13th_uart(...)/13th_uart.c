#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>


/*用于自动创建设备结点*/
static struct class *thirteenthdrv_class;  
static struct class_device	*thirteenthdrv_class_dev;

/* uart相关寄存器 */
volatile unsigned long *gphcon = NULL;
volatile unsigned long *gphup = NULL;

volatile unsigned long *ulcon0 = NULL;
volatile unsigned long *ucon0 = NULL;
volatile unsigned long *ubrdiv0 = NULL;
volatile unsigned long *utrstat0 = NULL;
volatile unsigned long *utxh0 = NULL;
volatile unsigned long *urxh0 = NULL;


static int thirteenth_drv_open(struct inode *inode, struct file *file)
{
	/* 配置GPH2,3为串口引脚，并且上拉 */
	*gphcon &= ~((0x3 << (2*2))|(0x3 << (3*2)));
	*gphcon |= ((0x2 << (2*2))|(0x2 << (3*2)));
	*gphup &= ~((0x1 << 2)|(0x1 << 3));

	/* 设置波特率 */
	*ubrdiv0 = 26;

	/* 设置模式 */
	*ucon0 = 0x0005;

	/* 设置数据格式 */
	*ulcon0 = 0x03;
		
	return 0;
}

/*发送一个字符函数*/
void put_one_char(unsigned char c)
{
	while(!(*utrstat0 & (1 << 2)));   //判断发送Buff是否为空，等待为空后向UTXH0写入要发送的内容
	*utxh0 = c;
}

/* 发送一串字符函数 */
void put_string(char *s)
{
	while(*s)
	{
		put_one_char(*s);
		s++;
	}
}

/* 接收一个字符函数 */
char getchar(void)
{
	while(!(*utrstat0 & (1 << 0)));
	return *urxh0;
}

static ssize_t thirteenth_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	/* 写函数里面发送应用层写入的数据 */
	char *s;
	copy_from_user(s, buf, count);
	printk("the write val from app: %s\n", s);
	//put_string(s);
	return 0;
}

static int thirteenth_drv_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	/* 读函数接收数据并返回给应用层 */
	char val;
	val = getchar();
	copy_to_user(buff, &val, sizeof(char));
	return sizeof(char);
}


static struct file_operations thirteenthdrv_fops = {
    .owner  =   THIS_MODULE,    
    .open   =   thirteenth_drv_open,     
	.read	=	thirteenth_drv_read,
	.write  =   thirteenth_drv_write,
};

int major;
static int thirteenth_drv_init(void)
{	
	major = register_chrdev(0, "thirteenth_drv", &thirteenthdrv_fops);

	thirteenthdrv_class = (struct class *)class_create(THIS_MODULE, "thirteenth_drv");
	thirteenthdrv_class_dev = (struct class_device *)class_device_create(thirteenthdrv_class, NULL, MKDEV(major, 0), NULL, "uart");

	gphcon = (volatile unsigned long *)ioremap(0x56000070, 16);
	gphup = gphcon + 2;
	
	ulcon0 = (volatile unsigned long *)ioremap(0x50000000, 24);
	ucon0 = ulcon0 + 1;
	utrstat0 = ulcon0 + 3;

	utxh0 = (volatile unsigned long *)ioremap(0x50000020, 16);
	urxh0 = utxh0 + 1;
	ubrdiv0 = utxh0 + 2;

	printk("uart load!\n");
	return 0;
}

static void thirteenth_drv_exit(void)
{
	unregister_chrdev(major, "thirteenth_drv");

	class_destroy(thirteenthdrv_class);
	class_device_unregister(thirteenthdrv_class_dev);

	iounmap(gphcon);
	iounmap(ulcon0);
	iounmap(utxh0);
}



module_init(thirteenth_drv_init);
module_exit(thirteenth_drv_exit);


MODULE_LICENSE("GPL");



























