#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/platform_device.h>

MODULE_AUTHOR("David Xie");
MODULE_LICENSE("Dual BSD/GPL");


static void my_device_release(struct device *dev)
{

}

static struct platform_device my_device = {
	.name = "my_platf_dev_hhh",  // shown dir. name at: /sys/bus/platform/devices/my_platfomr_device_hhh.0

	.dev={
		.release=my_device_release, // without release kernel will present a SERIOUS WARN!!
	     },

};


static __init int __init my_device_init(void)
{
	int ret = 0;

	printk(" ---- start init my platf_dev ----\n");

	if( &my_device == NULL)
	{
	     printk("----- platform_device *my_device == NULL! platform_device_register() fails, init exit.-----\n");
	     return -1;
	}

        ret=platform_device_register(&my_device);

	/*注册失败，释放相关内存*/
	if (ret)
	    platform_device_put(&my_device);

	return ret;
}

static __exit void my_device_exit(void)
{
	platform_device_unregister(&my_device);
	printk(" ---- my platf_dev exit ----\n");
}

module_init(my_device_init);
module_exit(my_device_exit);

