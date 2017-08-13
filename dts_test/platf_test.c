#include <linux/module.h>
#include <linux/init.h>
#include <linux/of_platform.h>

static struct of_device_id my_match_table[] = {
	{ .compatible = "midas,dts_test",     },
	{ },
};

static int my_probe(struct platform_device *dev)
{

    struct resource *res_mem;
    struct device_node *node_midas;
    int value;
    char *label;

    if( dev && (dev->name))
    {
        printk("--- dts_test: a compatibale device is found! -----");
	printk("matched platform_device:   name=%s    id=%d  num_resources=%d\n",dev->name,dev->id,dev->num_resources);

 	res_mem = platform_get_resource(dev,IORESOURCE_MEM,0);
	printk("get MEM resource: name=%s, start=0x%08x, end=0x%08x\n",res_mem->name,res_mem->start,res_mem->end); 

	//----- another way to get of_node is using: struct device_node *of_find_compatible_node() OR struct device_node *of_find_node_by_name()
	node_midas = dev->dev.of_node;
	//----- get u32 value of a property -----
	if(!of_property_read_u32(node_midas,"midas_value",&value))
		printk("value: %d ",value);
 	else
		printk("no property with name 'midas_value' \n");

	//----- get property of a label string -----
	label=(char *)of_get_property(node_midas,"label",NULL); /* return void  */
	if(label)
		printk("label: %s \n",label);
 	else
		printk("no property with name 'label' \n");

    }
    else
	printk("name unknwon \n");
    return 0;
}

static struct platform_driver my_driver = {
        .probe          = my_probe,
        .driver         = {
                .owner  = THIS_MODULE,
                .name   = "midas_drv",
		.of_match_table = my_match_table, //platform_bus will try to match .of_match_table first, and then .name 
        },
};

static int __init platf_test_init(void)
{

  printk(KERN_EMERG "------ dts v.s. platform_driver test start -----\n");

  /*注册平台驱动*/
  return platform_driver_register(&my_driver);

}

static void __exit platf_test_exit(void)
{

  /* 注销平台驱动 */
  platform_driver_unregister(&my_driver);

  printk(KERN_EMERG "------ dts v.s. platform driver test exit -----\n");

}

module_init(platf_test_init);
module_exit(platf_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MidasZhou");
MODULE_DESCRIPTION("dts_test");

