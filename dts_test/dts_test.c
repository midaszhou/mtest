/*------------------------------------------------
1.nano midas.dtsi  as:

/{
        midas {
                compatible = "midas,dts_test";
                midas_tip;
        };
};
2. add  #include "midas.dtsi"  to sun8i-h3-nanopi-neo.dts
3. make dtbs

---------------------------------------------------*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/of_platform.h>

static struct of_device_id my_match_table[] = {
	{ .compatible = "midas,dts_test",     },
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
        .driver         = {
                .owner  = THIS_MODULE,
                .name   = "midas_2",
		.of_match_table = my_match_table, //platform_bus will try to match .of_match_table first, and then .name 
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
MODULE_DESCRIPTION("dts_test");

