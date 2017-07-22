
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


static unsigned int gpio_int=203; //--linux num. for GP11
unsigned int gpio_beep=6;
unsigned int int_num=0;//--interrupter number

static struct work_struct int_wq; //--work queue for INT handler low part

static void enable_int_wq(struct work_struct *data)
{
   printk(KERN_INFO"Entering work_queue and start msleep....\n");
   msleep(1000);
   gpio_set_value(gpio_beep,0); //--stop beeping ... 
   printk(KERN_INFO"Re-enable irq now....\n");
   enable_irq(int_num);
}

static irqreturn_t int_handler(int irq, void *dev_id)
{

   disable_irq_nosync(int_num); //--!!!Must NOT use disable_irq() anyway,it's cause deadloop and will crash the kernel
   //---!!! to stop HW interrupt also

   printk(KERN_INFO "------ gpio_to_irq(PG11) triggered! -----\n");
   printk(KERN_INFO "Disable irq ...\n");
 
   gpio_set_value(gpio_beep,1); //--start to beep... 
   //----- irq handler low part -------
   schedule_work(&int_wq);

   return IRQ_HANDLED;
}


static int register_gpio_irq(void)
{
   int int_result=0;
   int_result=request_irq(int_num,int_handler,IRQF_TRIGGER_RISING,"GPIO_INT_Midas",(void *)&gpio_int);
   if(int_result != 0)
   {
	printk(KERN_EMERG "--- GPIO request irq failed! ---\n");
   }

   return int_result;
}

static int __init gpio_int_init(void)
{
   int ret=-1;

   printk(KERN_INFO"------ Start init gpio_int -----\n");

   gpio_direction_output(gpio_beep,0);
   if(gpio_is_valid(gpio_beep) == 0)
   {
	printk("gpio_beep is valid!\n");
	gpio_set_value(gpio_beep,0);
   }
   else
	printk("gpio_beep is NOT valid!\n");

   gpio_direction_input(gpio_int); //-set gpio direction
   if(gpio_is_valid(gpio_int) == 0)
   {
	printk("gpio_int is valid!\n");
	gpio_set_value(gpio_int,0);
   }
   else
	printk("gpio_int is NOT valid!\n");

//   gpio_set_value(gpio_int,0);

   int_num=gpio_to_irq(gpio_int);
   if(int_num>0)
	printk(KERN_INFO "------ gpio_to_irq(PG11):%d -----\n",int_num);
   else
	return -1;

   ret=register_gpio_irq();
   if(ret!= 0)
   {
	printk(KERN_EMERG "----- register_gpio_irq() failed! ----\n");
        return -1;
   }
   else
	//---init wor-queue for irq-handler low part
        INIT_WORK(&int_wq,enable_int_wq);

	return 0;
}

static void __exit gpio_int_exit(void)
{

  if(int_num > 0) 
  	free_irq(int_num,(void *)&gpio_int); //---!!! void *dev_id MUST be presented,eve no IRQF_SHARED flag in request_irq(xx)
  printk(KERN_EMERG "------ gpio interrupt test exit -----\n");

}

module_init(gpio_int_init);
module_exit(gpio_int_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux");
MODULE_DESCRIPTION("gpio-int-test");

