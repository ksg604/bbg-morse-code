/*
 * morsecode.c
 * Misc driver which flashes morse code
 * Author: Brandon Frison and Kevin San Gabriel
 *
 */


// Morsecode driver:
#include <linux/module.h>
#include <linux/miscdevice.h>

#define MY_DEVICE_FILE "morse-code"

static int morsecode_open(struct inode *inode, struct file *file){}
static int morsecode_close(struct inode *inode, struct file *file){}
static ssize_t morsecode_read(struct file *file, char*buff, size_t count, loff *ppos){}

struct file_operations morsecode_fops = {
	.owner	= THIS_MODULE,
	.open	= morsecode_open,
	.release = morsecode_close,
	.read	= morsecode_read
};

static struct miscdevice morsecode_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,	// Let system assign
	.name = MY_DEVICE_FILE,		// /dev/... file.
	.fops = &morsecode_fops		// Callback functions
};


static int __init morsecode_init(void)
{
	int returnValue;
	returnValue = misc_register(&morsecode_miscdevice);

	printk(KERN_INFO "---> Morse code driver init: file /dev/%s\n", MY_DEVICE_FILE);
	printk(KERN_INFO "---> Morse code driver has been successfully initialized.\n");

	return returnValue;
}

static void __exit morsecode_exit(void)
{
	printk(KERN_INFO "<--- Morse code driver is now exiting.\n");
	printk(KERN_INFO "<--- Morse code driver exit().\n");
	misc_deregister(&morsecode_miscdevice);
}

// Link init/exit functions into the kernel's code.

module_init(morsecode_init);
module_exit(morsecode_exit);

// Information about the module:
MODULE_AUTHOR("Kevin San Gabriel and Brandon Frison");
MODULE_DESCRIPTION("Driver which flashes morse code");
MODULE_LICENSE("GPL");
