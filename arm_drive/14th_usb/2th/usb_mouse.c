#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

/* 2.设置usb_driver结构体 */
/* 2.1设置id_table 支持哪些设备
 * HID类
 * BOOT子类
 * MOUSE协议
 */
static struct usb_device_id usbmouse_as_key_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },
	{ }	/* Terminating entry */
};

/* 2.2设置probe函数 */
static int usbmouse_as_key_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);  //通过usb_interface接口得到usb_device
	printk("found usbmouse!\n");
	printk("bcdUSB = %X\n", dev->descriptor.bcdUSB); //打印usb设备版本
	printk("VID    = 0x%X\n", dev->descriptor.idVendor); //打印usb设备厂家id
	printk("PID    = 0x%X\n", dev->descriptor.idProduct); //打印usb设备产品id

	return 0;
}


/* 2.3设置disconnect函数 */
static void usbmouse_as_key_disconnect(struct usb_interface *intf)
{
	printk("disconnect usbmouse!\n");
}


/* 1.分配一个usb_driver结构体变量 */
static struct usb_driver usbmouse_as_key_driver = {
	.name       = "usbmouse_as_key",
	.probe      = usbmouse_as_key_probe,
	.disconnect = usbmouse_as_key_disconnect,
	.id_table   = usbmouse_as_key_id_table,
};


static int usbmouse_as_key_init(void)
{
	/* 3.注册 */
	usb_register(&usbmouse_as_key_driver);
	return 0;
}

static void usbmouse_as_key_exit(void)
{
	/* 卸载 */
	usb_deregister(&usbmouse_as_key_driver);
}

module_init(usbmouse_as_key_init);
module_exit(usbmouse_as_key_exit);
MODULE_LICENSE("GPL");
















