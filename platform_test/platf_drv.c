#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/platform_device.h>

MODULE_AUTHOR("David Xie");
MODULE_LICENSE("Dual BSD/GPL");

static int my_probe(struct platform_device *dev)
{
    printk("--- Driver found device which my driver can handle!\n");
    return 0;
}

static int my_remove(struct platform_device *dev)
{
    printk("Driver found device unpluged!\n");
    return 0;
}

static struct platform_driver my_driver = {
	.probe		= my_probe,
	.remove		= my_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "my_platf_dev2_hh",
	},
};

static __init int __init my_driver_init(void)
{
        /*注册平台驱动*/
	return platform_driver_register(&my_driver);
}

static __exit void my_driver_exit(void)
{
	platform_driver_unregister(&my_driver);
	printk(" ----- platf_drv exits -----\n");
}

module_init(my_driver_init);
module_exit(my_driver_exit);
