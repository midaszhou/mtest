#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/gpio_keys.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <linux/interrupt.h> //--request_irq()
#include <linux/delay.h> //--msleep


static unsigned int gpio_num=203; //--linux num. for GP11
unsigned int int_num=0;//--interrupter number

static struct work_struct int_wq; //--work queue for INT handler low part

static void enable_int_wq(struct work_struct *data)
{
  printk(KERN_INFO"Entering work_queue and start msleep....\n");
  msleep_interruptible(300);
  printk(KERN_INFO"Re-enable irq  ...\n");
  enable_irq(int_num); 

}

static irqreturn_t int_handler(int irq, void *dev_id)
{
   printk(KERN_INFO "------ gpio_to_irq(PG11) triggered! -----\n");

   printk(KERN_INFO "Disable irq ...\n");
   disable_irq_nosync(int_num); //--!!!Must NOT use disable_irq() for shared IRQ.

   //----- irq handler low part -------
    schedule_work(&int_wq);
   
   return IRQ_HANDLED;
}


static int register_gpio_irq(void)
{
   int int_result=0;
   int_result=request_irq(int_num,int_handler,IRQF_TRIGGER_RISING,"GPIO_INT_Midas",(void *)&gpio_num);
   //---!!!! if IRQF_SHARED flag is set, then interrupt will be automattically trigged when the module is installed !!!!!!!!!
   if(int_result != 0)
   {
	printk(KERN_EMERG "--- GPIO request irq failed! ---\n");
   }

   return int_result;
}

static int __init dts_test_init(void)
{
  struct device_node *node=NULL;
  int ncount;

  char strCompatible[]="allwinner,sun8i-h3-r-pinctrl";

//  gpio_set_value(gpio_num,1);
  gpio_direction_input(gpio_num); //-set gpio direction
  msleep(300);
  int_num=gpio_to_irq(gpio_num);
  printk(KERN_INFO "------ gpio_to_irq(PG11):%d -----\n",int_num);
  if(int_num>0)
	 register_gpio_irq();

  //---init wor-queue for irq-handler low part
  INIT_WORK(&int_wq,enable_int_wq);

  printk(KERN_EMERG "------ device tree test start -----\n");

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

  if(int_num > 0) 
  	free_irq(int_num,(void *)&gpio_num); //---!!! void *dev_id MUST be presented,eve no IRQF_SHARED flag in request_irq(xx)

  printk(KERN_EMERG "------ device tree test exit -----\n");

}

module_init(dts_test_init);
module_exit(dts_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MidasZhou");
MODULE_DESCRIPTION("gpio-irq-test");

