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
#include <linux/delay.h>
#include <linux/leds.h>
#include <linux/kfifo.h>
#include <linux/uaccess.h>

#define MY_DEVICE_FILE "morse-code"

#define DOT_TIME 200
#define DASH_TIME 600
#define WORD_BREAK_TIME 1400

/***************************************
 * FIFO SUPPORT
 **************************************/
#define FIFO_SIZE 512	// Must be a power of 2.
static DECLARE_KFIFO(morse_fifo, char, FIFO_SIZE);

//TAKEN FROM SUGGESTED MORSE CODE ENCODINGS

// Morse Code Encodings (from http://en.wikipedia.org/wiki/Morse_code)
//   Encoding created by Brian Fraser. Released under GPL.
//
// Encoding description:
// - msb to be output first, followed by 2nd msb... (left to right)
// - each bit gets one "dot" time.
// - "dashes" are encoded here as being 3 times as long as "dots". Therefore
//   a single dash will be the bits: 111.
// - ignore trailing 0's (once last 1 output, rest of 0's ignored).
// - Space between dashes and dots is one dot time, so is therefore encoded
//   as a 0 bit between two 1 bits.
//
// Example:
//   R = dot   dash   dot       -- Morse code
//     =  1  0 111  0  1        -- 1=LED on, 0=LED off
//     =  1011 101              -- Written together in groups of 4 bits.
//     =  1011 1010 0000 0000   -- Pad with 0's on right to make 16 bits long.
//     =  B    A    0    0      -- Convert to hex digits
//     = 0xBA00                 -- Full hex value (see value in table below)
//
// Between characters, must have 3-dot times (total) of off (0's) (not encoded here)
// Between words, must have 7-dot times (total) of off (0's) (not encoded here).
//
static unsigned short morsecode_codes[] = {
		0xB800,	// A 1011 1
		0xEA80,	// B 1110 1010 1
		0xEBA0,	// C 1110 1011 101
		0xEA00,	// D 1110 101
		0x8000,	// E 1
		0xAE80,	// F 1010 1110 1
		0xEE80,	// G 1110 1110 1
		0xAA00,	// H 1010 101
		0xA000,	// I 101
		0xBBB8,	// J 1011 1011 1011 1
		0xEB80,	// K 1110 1011 1
		0xBA80,	// L 1011 1010 1
		0xEE00,	// M 1110 111
		0xE800,	// N 1110 1
		0xEEE0,	// O 1110 1110 111
		0xBBA0,	// P 1011 1011 101
		0xEEB8,	// Q 1110 1110 1011 1
		0xBA00,	// R 1011 101
		0xA800,	// S 1010 1
		0xE000,	// T 111
		0xAE00,	// U 1010 111
		0xAB80,	// V 1010 1011 1
		0xBB80,	// W 1011 1011 1
		0xEAE0,	// X 1110 1010 111
		0xEBB8,	// Y 1110 1011 1011 1
		0xEEA0	// Z 1110 1110 101
};

/***************************************
 * LED Trigger
 **************************************/
DEFINE_LED_TRIGGER(my_trigger);

static void morse_led_blink_on(void) {
	led_trigger_event(my_trigger, LED_FULL);
}

static void morse_led_off(void) 
{
	led_trigger_event(my_trigger, LED_OFF);
}

static void led_register(void)
{
	// Setup the trigger's name:
	led_trigger_register_simple("morse-code", &my_trigger);
}

static void led_unregister(void)
{
	// Cleanup
	led_trigger_unregister_simple(my_trigger);
}

/***************************************
 * Callback functions
 **************************************/

static ssize_t morsecode_read(struct file *file, char*buff, size_t count, loff_t *ppos) 
{

	int num_bytes_read = 0;
	/*
	char val;

	// Check if fifo is empty
	
	
	if ( !kfifo_get(&morse_fifo, &val) ) {
		// If empty, return 0 bytes read
		return 0;
	}*/

	// Copy data from fifo to user space
	if ( kfifo_to_user(&morse_fifo, buff, count, &num_bytes_read) ) {
		return -EFAULT;
	}
	
	return num_bytes_read;
}

static ssize_t morsecode_write(struct file *file, const char *buff, size_t count, loff_t *ppos)
{
	int i, idx, bitIndex, numBits;
	unsigned short code, bit, threeBits;

	numBits = 8 * sizeof(code);
	bitIndex = 0;

	for (i = 0; i < count; i++) {
		char letter;

		if ( i == (count-1 ) ) {
			//Reached end of transmission, add a line feed \n to the end of the queue
			if(!kfifo_put(&morse_fifo, '\n')) {
				//Fifo full should be unnecessary as we made it 512
				return -EFAULT;
			}
			break;
		}

		if (copy_from_user(&letter, &buff[i], sizeof(letter))) {
			return -EFAULT;
		}

		code = 0U;
		if (letter == ' ') {
			// If the is a word break, add two extra spaces to the queue (for a total of 3) between words
			if(!kfifo_put(&morse_fifo, ' ')) {
				return -EFAULT;
			}
			if(!kfifo_put(&morse_fifo, ' ')) {
				return -EFAULT;
			}
			//sleep for word break
			msleep(WORD_BREAK_TIME);
			continue;
			// need to ignore case sensitivity now.
		} else if (65 <= letter && letter <= 90) { // 65 to 90 are the codes for Capital letters A-Z
			code = morsecode_codes[letter - 65];
		} else if (97 <= letter && letter <= 122) { // 97 to 122 are the codes for Small letters a-z
			code = morsecode_codes[letter - 97];
		}

		// blink if the code is not 0 or basically if any character other than the letters and space
		if (code) {
			for (idx = 0; idx < numBits; idx++) {
				bit = (code & ( 1 << idx )) >> idx;
				//index of last bit so next part only grabs correct sequence
				if (bit) {
					bitIndex = idx;
					break;
				}
			}

			// read the entire bit sequence
			// 7 is 111
			for (idx = numBits - 1; idx >= bitIndex; idx--) {
				threeBits = (code & (7 << (idx-2))) >> (idx-2);
				bit = (code & ( 1 << idx )) >> idx;
	
				if (threeBits == 7) {
					if (!kfifo_put(&morse_fifo, '-')) {
						return -EFAULT;
					}	
					morse_led_blink_on();
					msleep(DASH_TIME);
					idx -= 2;
					continue;
				} else if (bit) {
					// 1
					if (!kfifo_put(&morse_fifo, '.')) {
						return -EFAULT;
					}
					morse_led_blink_on();
				} else {
					// 0
					morse_led_off();
				}

				msleep(DOT_TIME);
			}


			// turn led off then sleep for 3*dot time or dashtime.
			if ( i < (count-2) ) {
				if (!kfifo_put(&morse_fifo, ' ')) {
					return -EFAULT;
				}
			}
			morse_led_off();
			msleep(DASH_TIME);
		}
	}

	*ppos += count;
	return count;

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
	INIT_KFIFO(morse_fifo);
	returnValue = misc_register(&morsecode_miscdevice);

	printk(KERN_INFO "---> Morse code driver init: file /dev/%s\n", MY_DEVICE_FILE);
	printk(KERN_INFO "---> Morse code driver has been successfully initialized.\n");

	led_register();


	return returnValue;
}

static void __exit morsecode_exit(void)
{
	printk(KERN_INFO "<--- Morse code driver is now exiting.\n");
	printk(KERN_INFO "<--- Morse code driver exit().\n");
	misc_deregister(&morsecode_miscdevice);

	led_unregister();
}

// Link init/exit functions into the kernel's code.

module_init(morsecode_init);
module_exit(morsecode_exit);

// Information about the module:
MODULE_AUTHOR("Kevin San Gabriel and Brandon Frison");
MODULE_DESCRIPTION("Driver which flashes morse code");
MODULE_LICENSE("GPL");
