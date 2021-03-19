/*
 * morsecode.c
 * Misc driver which flashes morse code
 * Author: Brandon Frison and Kevin San Gabriel
 *
 */


// Morsecode driver:
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

#define MY_DEVICE_FILE "morse-code"

/***************************************
 * Callback functions
 **************************************/

//static int morsecode_open(struct inode *inode, struct file *file);
//static int morsecode_close(struct inode *inode, struct file *file);
static ssize_t morsecode_read(struct file *file, char*buff, size_t count, loff_t *ppos) 
{
	printk(KERN_INFO "morsecode: In morsecode_read(): buffer size %d, f_pos %d\n",
		(int) count, (int) *ppos);
	return 0;
}
static ssize_t morsecode_write(struct file *file, const char *buff, size_t count, loff_t *ppos)
{
	printk(KERN_INFO "morsecode: In morsecode_write(): ");

	return 0;

}


/****************************************
 * Misc support
 ***************************************/
struct file_operations morsecode_fops = {
	.owner	= THIS_MODULE,
	.read	= morsecode_read,
	.write	= morsecode_write
};

static struct miscdevice morsecode_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,	// Let system assign
	.name = MY_DEVICE_FILE,		// /dev/... file.
	.fops = &morsecode_fops		// Callback functions
};



/***************************************
 * Driver initialization and exit:
 ***************************************/
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
