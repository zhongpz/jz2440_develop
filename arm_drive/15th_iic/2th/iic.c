#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/mutex.h>

static unsigned short ignore[]      = { I2C_CLIENT_END };   //忽略
static unsigned short normal_addr[] = { 0x50, I2C_CLIENT_END }; /* 地址值是7位 */

static unsigned short force_addr[] = {ANY_I2C_BUS, 0x60, I2C_CLIENT_END};
static unsigned short * forces[] = {force_addr, NULL};


static struct i2c_client_address_data addr_data = {
	.normal_i2c	= ignore,  /* 要发出S信号和设备地址并得到ACK信号,才能确定存在这个设备 */
	.probe		= ignore,
	.ignore		= ignore,
	.forces   	= forces, /* 强制认为存在这个设备 */
};


/* 有对应设备就执行这个函数 */
static int iic_detect(struct i2c_adapter *adapter, int address, int kind)
{
	printk("iic_detect\n");
	return 0;
}

/* 注册i2c_driver时调用 */
static int iic_attach(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, iic_detect);
}

/* 如果有对应设备，则在卸载时调用 */
static int iic_detach(struct i2c_client *client)
{
	printk("iic_detach\n");
	return 0;
}

/* 1.分配一个i2c_driver结构体 */
/* 2.设置i2c_driver结构体 */

static struct i2c_driver iic_driver = {
	.driver = {
		.name = "iic_test",
	},
	/* .id = 可能没用 */
	.attach_adapter = iic_attach,   //注册i2c_driver结构体时调用
	.detach_client = iic_detach,    //如果有对应iic设备，则卸载时调用
};


static int iic_init(void)
{
	/* 3.注册 */
	i2c_add_driver(&iic_driver);
	return 0;
}

static void iic_exit(void)
{
	i2c_del_driver(&iic_driver);
}
module_init(iic_init);
module_exit(iic_exit);
MODULE_LICENSE("GPL");













