#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>


static struct input_dev *uk_dev;
static struct urb *uk_urb;
static dma_addr_t usb_buf_phys;
static char *usb_buf;
static int len;

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

static void usbmouse_as_key_irq(struct urb *urb)
{
	static unsigned char pre_val;
	#if 0
	int i;
	static int cnt = 0;
	printk("data cnt %d: ", ++cnt);
	for (i = 0; i < len; i++)
	{
		printk("%02x ", usb_buf[i]);
	}
	printk("\n");
	#endif
	/* USB鼠标数据含义
	 * usb_buf[0]: bit0-左键, 1-按下, 0-松开
	 *             bit1-右键, 1-按下, 0-松开
	 *             bit2-中键, 1-按下, 0-松开 
	 *
     */
	
	if ((pre_val & (1<<0)) != (usb_buf[0] & (1<<0)))
	{
		/* 左键发生了变化 */
		input_event(uk_dev, EV_KEY, KEY_L, (usb_buf[0] & (1<<0)) ? 1 : 0);
		input_sync(uk_dev);
	}

	if ((pre_val & (1<<1)) != (usb_buf[0] & (1<<1)))
	{
		/* 右键发生了变化 */
		input_event(uk_dev, EV_KEY, KEY_S, (usb_buf[0] & (1<<1)) ? 1 : 0);
		input_sync(uk_dev);
	}
	
	if ((pre_val & (1<<2)) != (usb_buf[0] & (1<<2)))
	{
		/* 中键发生了变化 */
		input_event(uk_dev, EV_KEY, KEY_ENTER, (usb_buf[0] & (1<<2)) ? 1 : 0);
		input_sync(uk_dev);
	}
		
	pre_val = usb_buf[0];
	
	/* 重新提交urb */
	usb_submit_urb(uk_urb, GFP_KERNEL);
}


/* 2.2设置probe函数 */
static int usbmouse_as_key_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);  //通过usb_interface接口得到usb_device
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	int pipe;

	interface = intf->cur_altsetting;
	endpoint = &interface->endpoint[0].desc;
	
	
	printk("found usbmouse!\n");
	printk("bcdUSB = %X\n", dev->descriptor.bcdUSB); //打印usb设备版本
	printk("VID    = 0x%X\n", dev->descriptor.idVendor); //打印usb设备厂家id
	printk("PID    = 0x%X\n", dev->descriptor.idProduct); //打印usb设备产品id

	/* 使用输入子系统实现鼠标用着键盘 */
	/* a.分配一个input_dev结构体 */
	uk_dev = input_allocate_device();

	/* b.设置input_dev结构体 */
	set_bit(EV_KEY, uk_dev->evbit);  //能产生按键类事件
	set_bit(EV_REP, uk_dev->evbit);  //能产生重复类事件

	set_bit(KEY_L, uk_dev->keybit);
	set_bit(KEY_S, uk_dev->keybit);
	set_bit(KEY_ENTER, uk_dev->keybit);

	/* c.注册input_dev结构体 */
	input_register_device(uk_dev);

	/* d.硬件相关操作
	 * 以前的驱动程序数据通过读寄存器，引脚状态而来
	 * usb驱动的数据通过底层的usb总线驱动程序提供的函数来收发
	 * 记住数据传输三要素：源，目的，地址
	 * 源：  usb设备的某个端点
	 * 目的：从 USB 设备读数据，读到一个缓冲区，
	 * 长度：端点描述符中有长度.usb_endpoint_descriptor.wMaxPacketSize(最大包大小)
	 */

	/* 源 */
	pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);

	/* 长度 */
	len = endpoint->wMaxPacketSize;

	/* 目的 */
	usb_buf = usb_buffer_alloc(dev, len, GFP_ATOMIC, &usb_buf_phys); 

	/* 分配usb request block */
	uk_urb = usb_alloc_urb(0, GFP_KERNEL);
	
	/* 使用三要素设置urb */
	usb_fill_int_urb(uk_urb, dev, pipe, usb_buf, len, usbmouse_as_key_irq, NULL, endpoint->bInterval);
	uk_urb->transfer_dma = usb_buf_phys;
	uk_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP; 

	/* 使用urb */
	usb_submit_urb(uk_urb, GFP_KERNEL);
	
	return 0;
}


/* 2.3设置disconnect函数 */
static void usbmouse_as_key_disconnect(struct usb_interface *intf)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	//printk("disconnect usbmouse!\n");
	usb_kill_urb(uk_urb);
	usb_free_urb(uk_urb);

	usb_buffer_free(dev, len, usb_buf, usb_buf_phys);
	input_unregister_device(uk_dev);
	input_free_device(uk_dev);
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
















