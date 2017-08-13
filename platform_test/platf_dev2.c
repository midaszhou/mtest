#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/platform_device.h>

MODULE_AUTHOR("David Xie");
MODULE_LICENSE("Dual BSD/GPL");

//---- here release is not necessary.
static void my_device_release(struct device *dev)
{

}

static struct platform_device *my_device;


static __init int __init my_device_init(void)
{
	int ret = 0;

	printk(" ---- start init my platf_dev2 ----\n");

        // 分配结构 
	my_device = platform_device_alloc("my_platf_dev2_hh", -1); //show dir. name at: /sys/bus/platform/devices/my_platform_dev

        // 注册设备
	ret = platform_device_add(my_device);

	if( my_device == NULL)
	{
	     printk("----- platform_device *my_device == NULL! platform_device_register() fails, init exit.-----\n");
	     return -1;
	}

	/*注册失败，释放相关内存*/
	if (ret)
	    platform_device_put(my_device);

	return ret;
}

static __exit void my_device_exit(void)
{
	platform_device_unregister(my_device);
	printk(" ---- my platf_dev2 exit ----\n");
}

module_init(my_device_init);
module_exit(my_device_exit);

