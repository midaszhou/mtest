#include <linux/module.h>
#include <linux/init.h>
//#include <linux/fs.h>
//#include <linux/gpio_keys.h>
#include <linux/of_platform.h>
//#include <linux/device.h>
//#include <linux/of_gpio.h>
//#include <linux/spinlock.h>
//#include <linux/gpio.h>
//#include <linux/interrupt.h> //--request_irq()
//#include <linux/delay.h> //--msleep

static struct of_device_id my_match_table[] = {
	{ .compatible = "allwinner,sun5i-a13-ir",     },
	{ },
};


static int my_probe(struct platform_device *dev)
{
    if( dev && (dev->name))
    {
        printk("--- dts_test: a compatibale device is found! -----");
	printk("matched platform_device:   name=%s    id=%d  num_resources=%d\n",dev->name,dev->id,dev->num_resources);
    }
    else
	printk("name unknwon =n");
    return 0;
}



static struct platform_driver my_driver = {
        .probe          = my_probe,
//        .remove         = my_remove,
        .driver         = {
                .owner  = THIS_MODULE,
                .name   = "ir",//"my_platf_dev2_hh",
		.of_match_table = my_match_table,
        },
};

static int __init dts_test_init(void)
{
  struct device_node *node=NULL;
  int ncount;
  char strCompatible[]="allwinner,sun8i-h3-r-pinctrl";

  printk(KERN_EMERG "------ device tree test start -----\n");

  /*注册平台驱动*/
  return platform_driver_register(&my_driver);


  node=of_find_compatible_node(NULL,NULL,strCompatible);
  if(node != NULL)
  {
      ncount=of_get_child_count(node);
      printk(KERN_EMERG "%d nodes found with compatible string '%s'\n",ncount,strCompatible);
  }
  else
      printk(KERN_EMERG "No nodes found with compatible string: '%s'\n",strCompatible);

  return 0;
}

static void __exit dts_test_exit(void)
{
  /* 注销平台驱动 */
  platform_driver_unregister(&my_driver);

  printk(KERN_EMERG "------ device tree test exit -----\n");

}

module_init(dts_test_init);
module_exit(dts_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MidasZhou");
MODULE_DESCRIPTION("gpio-irq-test");

