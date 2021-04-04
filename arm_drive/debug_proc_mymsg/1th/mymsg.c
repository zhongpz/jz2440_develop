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
#include <linux/proc_fs.h>

struct proc_dir_entry *myentry;

const struct file_operations proc_mykmsg_operations = {

};


static int mymsg_init(void)
{
	myentry = create_proc_entry("mymsg", S_IRUSR, &proc_root);   //创建条目
	if (myentry)
		myentry->proc_fops = &proc_mykmsg_operations;   //file_operation结构
	return 0;
}


static void mymsg_exit(void)
{
	remove_proc_entry("mymsg", &proc_root);
}

module_init(mymsg_init);
module_exit(mymsg_exit);
MODULE_LICENSE("GPL");




