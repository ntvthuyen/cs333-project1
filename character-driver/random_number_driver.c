
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>

#define DRIVER_AUTHOR "Nguyen Ho Huu Nghia <huunghia160799@gmail.com>"
#define DRIVER_DESC "A sample character device driver"
#define DRIVER_VERSION "0.3"

struct vchar_driver {
	dev_t device_number;
	struct class *device_class;
	struct device *device;
} random_number_driver;

/****************************** device specific - START *****************************/
/* ham khoi tao thiet bi */

/* ham giai phong thiet bi */

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
	int register_result;
	// random_number_driver.device_number = MKDEV(69, 0);
	random_number_driver.device_number = 0;
	register_result = alloc_chrdev_region(&random_number_driver.device_number, 0, 1, "random_number_driver");
	if (register_result < 0) {
		printk("Failed to register device number\n");
		return register_result;
	}
	printk("Allocated device number (%d, %d)\n", MAJOR(random_number_driver.device_number), MINOR(random_number_driver.device_number));

	/* create device file */
	random_number_driver.device_class = class_create(THIS_MODULE, "class_vchar_dev");
	if (random_number_driver.device_class == NULL) {
		printk("Failed to create a device class\n");
		unregister_chrdev_region(random_number_driver.device_number, 1);
	}

	random_number_driver.device = device_create(random_number_driver.device_class, NULL, random_number_driver.device_number, NULL, "vchar_dev");

	if (IS_ERR(random_number_driver.device)) {
		printk("Failed to create a device\n");
		class_destroy(random_number_driver.device_class);
	}

	/* cap phat bo nho cho cac cau truc du lieu cua driver va khoi tao */

	/* khoi tao thiet bi vat ly */

	/* dang ky cac entry point voi kernel */

	/* dang ky ham xu ly ngat */

	printk("Initialize random number driver successfully\n");
	return 0;
}

/* ham ket thuc driver */
static void __exit vchar_driver_exit(void)
{
	/* huy dang ky xu ly ngat */

	/* huy dang ky entry point voi kernel */

	/* giai phong thiet bi vat ly */

	/* giai phong bo nho da cap phat cau truc du lieu cua driver */

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