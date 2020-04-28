#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "random_number_driver.h"

#define DRIVER_AUTHOR "Nguyen Ho Huu Nghia <huunghia160799@gmail.com>"
#define DRIVER_DESC "A sample character device driver"
#define DRIVER_VERSION "0.6"

// The class for the character device
typedef struct vchar_dev {
	unsigned char *control_registers;
	unsigned char *status_registers;
	unsigned char *data_registers;
} vchar_device_t;

// The main class for the driver
struct vchar_driver {
	dev_t device_number; // device number consist of major and minor number
	struct class *device_class; // the device class
	struct device *device; // the device
	vchar_device_t *vchar_hardware; // the pointer to the underlying character device struct
	struct cdev *vcdev; //  the pointer to the character device (cdev is supplied by the library)
	unsigned int open_cnt; // the number of open entry points
} random_number_driver;

// generate a random number as a char array
char *random(void)
{
	const int MAX_LENGTH = 11;
	int i;
	// generate random number as integer
	get_random_bytes(&i, sizeof(i));
	printk("%d", i);

	// count the number of digits (including '-')
	int t = i, c = 0;
	if (t < 0) {
		t = -t;
		c = 1;
	}
	while (t > 0) {
		t = t / 10;
		++c;
	}

	// allocate string
	char *str = kzalloc(MAX_LENGTH, GFP_KERNEL);
	int index = 0;

	// pad space at the start
	int remaining = MAX_LENGTH - c, j = 0;
	while (j++ < remaining) {
		str[index++] = ' ';
	}

	// add '-' (if applicable)
	if (i < 0) {
		str[index] = '-';
		i = i * -1;
		++index;
	}

	// extract the digits (these digits are in the wrong direction)
	char *temp = kzalloc(c + 1, GFP_KERNEL);
	int temp_index = 0;
	while (i > 0) {
		temp[temp_index] = (char)(i % 10 + '0');
		i = i / 10;
		++temp_index;
	}

	// revert the digits to the right direction
	int k = temp_index - 1;
	while (k >= 0) {
		str[index++] = temp[k--];
	}

	printk("Final random number: %s", str);
	return str;
}

/****************************** device specific - START *****************************/
/* initialize the character device */
int vchar_hw_init(vchar_device_t *hardware)
{
	char *buffer;
	buffer = kzalloc(NUM_DEV_REGS * REG_SIZE, GFP_KERNEL);
	if (!buffer) {
		return -ENOMEM;
	}

	hardware->control_registers = buffer;
	hardware->status_registers = hardware->control_registers + NUM_CTRL_REGS;
	hardware->data_registers = hardware->status_registers + NUM_STS_REGS;

	// initialize the registers
	hardware->control_registers[CONTROL_ACCESS_REG] = 0x03;
	hardware->status_registers[DEVICE_STATUS_REG] = 0x03;

	return 0;
}

/* what to do when the device is destroyed */
void vchar_hw_exit(vchar_device_t *hardware)
{
	kfree(hardware->control_registers);
}

/* read the data from the device's registers, put that in the buffer and then return the number of bytes read */
int vchar_hw_read_data(vchar_device_t *hw, int start_register, int num_registers, char *kbuf)
{
	int number_bytes_read = num_registers;

	if ((hw->control_registers[CONTROL_ACCESS_REG] & CTRL_READ_DATA_BIT) == DISABLED)
		return -1;

	if (kbuf == NULL)
		return -1;

	if (start_register > NUM_DATA_REGS)
		return -1;

	if (num_registers > (NUM_DATA_REGS - start_register))
		number_bytes_read = NUM_DATA_REGS - start_register;

	char *number = random();
	memcpy(kbuf, number, number_bytes_read);

	// memcpy(kbuf, hw->data_registers + start_register, number_bytes_read);

	hw->status_registers[READ_COUNT_L_REG]++;
	if (hw->status_registers[READ_COUNT_L_REG] == 0)
		hw->status_registers[READ_COUNT_H_REG]++;

	return number_bytes_read;
}

/* write the data from the kernel buffer to registers, then return the number bytes written */
int vchar_hw_write_data(vchar_device_t *hw, int start_register, int num_registers, char *kbuf)
{
	int number_bytes_written = num_registers;

	if ((hw->control_registers[CONTROL_ACCESS_REG] & CTRL_WRITE_DATA_BIT) == DISABLED)
		return -1;

	if (kbuf == NULL)
		return -1;

	if (start_register > NUM_DATA_REGS)
		return -1;

	if (num_registers > (NUM_DATA_REGS - start_register)) {
		number_bytes_written = NUM_DATA_REGS - start_register;
		hw->status_registers[DEVICE_STATUS_REG] |= STS_DATAREGS_OVERFLOW_BIT;
	}

	memcpy(hw->data_registers + start_register, kbuf, number_bytes_written);

	hw->status_registers[WRITE_COUNT_L_REG]++;
	if (hw->status_registers[WRITE_COUNT_L_REG] == 0) {
		hw->status_registers[WRITE_COUNT_H_REG]++;
	}

	return number_bytes_written;
}

/******************************* device specific - END *****************************/



/******************************** OS specific - START *******************************/
/* entry points function */

static int vchar_driver_open(struct inode *inode, struct file *filp)
{
	random_number_driver.open_cnt++;
	printk("Handled opened event (%d)\n", random_number_driver.open_cnt);
	return 0;
}

static int vchar_driver_release(struct inode *inode, struct file *filp)
{
	random_number_driver.open_cnt--;
	printk("Handled closed event\n");
	return 0;
}

static ssize_t vchar_driver_read(struct file *filp, char __user *user_buffer, size_t len, loff_t *off)
{
	char *kernel_buffer = NULL;
	int num_bytes = 0;

	printk("Handle read event start from %lld, %zu bytes\n", *off, len);

	kernel_buffer = kzalloc(len, GFP_KERNEL);
	if (kernel_buffer == NULL)
		return 0;
	num_bytes = vchar_hw_read_data(random_number_driver.vchar_hardware, *off, len, kernel_buffer);
	printk("read %d bytes from hardware\n", num_bytes);

	if (num_bytes < 0)
		return -EFAULT;
	if (copy_to_user(user_buffer, kernel_buffer, num_bytes))
		return -EFAULT;

	*off += num_bytes;
	return num_bytes;
}

static ssize_t vchar_driver_write(struct file *filp, const char __user *user_buffer, size_t len, loff_t *off)
{
	char *kernel_buffer = NULL;
	int num_bytes = 0;

	printk("Handle write event start from %lld for the size of %zu bytes\n", *off, len);

	kernel_buffer = kzalloc(len, GFP_KERNEL);

	if (copy_from_user(kernel_buffer, user_buffer, len))
		return -EFAULT;

	num_bytes = vchar_hw_write_data(random_number_driver.vchar_hardware, *off, len, kernel_buffer);
	printk("Write %d bytes to hardware\n", num_bytes);

	if (num_bytes < 0)
		return -EFAULT;

	*off += num_bytes;
	return num_bytes;
}

// mounting the entry functions above to the corresponding system calls
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = vchar_driver_open,
	.release = vchar_driver_release,
	.read = vchar_driver_read,
	.write = vchar_driver_write,
};

/* initializing the driver */
static int __init vchar_driver_init(void)
{
	/* register device number */
	int result;
	// random_number_driver.device_number = MKDEV(69, 0);
	random_number_driver.device_number = 0;
	result = alloc_chrdev_region(&random_number_driver.device_number, 0, 1, "random_number_driver");
	if (result < 0) {
		printk("Failed to register device number\n");
		goto failed_register_devnum;
	}
	printk("Allocated device number (%d, %d)\n", MAJOR(random_number_driver.device_number), MINOR(random_number_driver.device_number));

	/* create device file */
	random_number_driver.device_class = class_create(THIS_MODULE, "class_vchar_dev");
	if (random_number_driver.device_class == NULL) {
		printk("Failed to create a device class\n");
		goto failed_create_class;
	}

	random_number_driver.device = device_create(random_number_driver.device_class, NULL, random_number_driver.device_number, NULL, "random_number_char_dev");

	if (IS_ERR(random_number_driver.device)) {
		printk("Failed to create a device\n");
		goto failed_create_device;
	}

	/* allocate the memory for the data structure used by the driver */
	random_number_driver.vchar_hardware = kzalloc(sizeof(vchar_device_t), GFP_KERNEL);
	if (!random_number_driver.vchar_hardware) {
		printk("Failed to allocate memory for the data structure of the driver\n");
		goto failed_allocate_structure;
	}

	/* init the hardware of the character device */
	result = vchar_hw_init(random_number_driver.vchar_hardware);
	if (result < 0) {
		printk("Failed to initialize the virtual character device hardware\n");
		goto failed_init_hw;
	}

	/* allocate the memory for vcdev, then register the entry points to the OS */
	random_number_driver.vcdev = cdev_alloc();
	if (random_number_driver.vcdev == NULL) {
		printk("Failed to allocate cdev data structure\n");
		goto failed_allocate_cdev;
	}
	cdev_init(random_number_driver.vcdev, &fops);
	result = cdev_add(random_number_driver.vcdev, random_number_driver.device_number, 1);
	if (result < 0) {
		printk("Failed to add char device to the system\n");
		goto failed_allocate_cdev;
	}

	printk("Initialize random number driver successfully\n");
	return 0;

failed_allocate_cdev:
	vchar_hw_exit(random_number_driver.vchar_hardware);
failed_init_hw:
	kfree(random_number_driver.vchar_hardware);
failed_allocate_structure:
	device_destroy(random_number_driver.device_class, random_number_driver.device_number);
failed_create_device:
	class_destroy(random_number_driver.device_class);
failed_create_class:
	unregister_chrdev_region(random_number_driver.device_number, 1);
failed_register_devnum:
	return result;
}

/* what to free when the driver exits */
static void __exit vchar_driver_exit(void)
{
	/* delete entry points registered with the OS */
	cdev_del(random_number_driver.vcdev);

	/* free the hardware */
	vchar_hw_exit(random_number_driver.vchar_hardware);

	/* free the memory for the driver's data structure */
	kfree(random_number_driver.vchar_hardware);

	/* destroy the device file as well as the device file class */
	device_destroy(random_number_driver.device_class, random_number_driver.device_number);
	class_destroy(random_number_driver.device_class);

	/* unregister the device number */
	unregister_chrdev_region(random_number_driver.device_number, 1);

	printk("Exit random number driver\n");
}
/********************************* OS specific - END ********************************/

module_init(vchar_driver_init);
module_exit(vchar_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_SUPPORTED_DEVICE("testdevice");