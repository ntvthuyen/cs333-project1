#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "random_number_driver.h"

#define DRIVER_AUTHOR "Nguyen Ho Huu Nghia <huunghia160799@gmail.com>"
#define DRIVER_DESC "A sample character device driver"
#define DRIVER_VERSION "0.4"

typedef struct vchar_dev {
	unsigned char *control_registers;
	unsigned char *status_registers;
	unsigned char *data_registers;
} vchar_device_t;

struct vchar_driver {
	dev_t device_number;
	struct class *device_class;
	struct device *device;
	vchar_device_t *vchar_hardware;
} random_number_driver;

/****************************** device specific - START *****************************/
/* ham khoi tao thiet bi */
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

/* ham giai phong thiet bi */
void vchar_hw_exit(vchar_device_t *hardware)
{
	kfree(hardware->control_registers);
}

/* ham doc tu cac thanh ghi du lieu cua thiet bi */

/* ham ghi vao cac thanh ghi du lieu cua thiet bi */

/* ham doc tu cac thanh ghi trang thai cua thiet bi */

/* ham ghi vao cac thanh ghi dieu khien cua thiet bi */

/* ham xu ly tin hieu ngat gui tu thiet bi */

/******************************* device specific - END *****************************/

/******************************** OS specific - START *******************************/
/* cac ham entry points */

/* ham khoi tao driver */
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

	random_number_driver.device = device_create(random_number_driver.device_class, NULL, random_number_driver.device_number, NULL, "vchar_dev");

	if (IS_ERR(random_number_driver.device)) {
		printk("Failed to create a device\n");
		goto failed_create_device;
	}

	/* cap phat bo nho cho cac cau truc du lieu cua driver va khoi tao */
	random_number_driver.vchar_hardware = kzalloc(sizeof(vchar_device_t), GFP_KERNEL);
	if (!random_number_driver.vchar_hardware) {
		printk("Failed to allocate memory for the data structure of the driver\n");
		goto failed_allocate_structure;
	}

	/* khoi tao thiet bi vat ly */
	result = vchar_hw_init(random_number_driver.vchar_hardware);
	if (result < 0) {
		printk("Failed to initialize the virtual character device hardware\n");
		goto failed_init_hw;
	}

	/* dang ky cac entry point voi kernel */

	/* dang ky ham xu ly ngat */

	printk("Initialize random number driver successfully\n");
	return 0;

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

/* ham ket thuc driver */
static void __exit vchar_driver_exit(void)
{
	/* huy dang ky xu ly ngat */

	/* huy dang ky entry point voi kernel */

	/* giai phong thiet bi vat ly */
	vchar_hw_exit(random_number_driver.vchar_hardware);

	/* giai phong bo nho da cap phat cau truc du lieu cua driver */
	kfree(random_number_driver.vchar_hardware);

	/* xoa bo device file */
	device_destroy(random_number_driver.device_class, random_number_driver.device_number);
	class_destroy(random_number_driver.device_class);

	/* giai phong device number */
	unregister_chrdev_region(random_number_driver.device_number, 1);

	printk("Exit random number driver\n");
}
/********************************* OS specific - END ********************************/

module_init(vchar_driver_init);
module_exit(vchar_driver_exit);

MODULE_LICENSE("GPL");				   /* giay phep su dung cua module */
MODULE_AUTHOR(DRIVER_AUTHOR);		   /* tac gia cua module */
MODULE_DESCRIPTION(DRIVER_DESC);	   /* mo ta chuc nang cua module */
MODULE_VERSION(DRIVER_VERSION);		   /* mo ta phien ban cuar module */
MODULE_SUPPORTED_DEVICE("testdevice"); /* kieu device ma module ho tro */