
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

#define CPU_PORT_CONTROL_BASE 0x01C20800
#define PG_CFG1_REG 0xDC
#define PG_DRV0_REG 0xEC
#define PG_PULL0_REG 0xF4
#define PG_EINT_CFG1_REG 0x224
#define PG_EINT_CTL_REG  0x230
#define PG_EINT_STATUS_REG 0x234
#define PG_CONTROL_IOMAP_SIZE 0x23C

volatile unsigned long phys_addr,virt_addr; //-- physical and virtual address 

static unsigned int gpio_int=203; //--linux GPIO number of PG11,as for GPIO Interrupt test.
unsigned int gpio_beep=6; //---linux GPIO number of PA6, as for beeper
unsigned int int_num=0;//--interrupter number

struct timespec tv;

static struct work_struct int_wq; //--work queue for INT handler low part

/*------  map IO addr. -------*/
static void map_gpio_reg(void)
{
   phys_addr= CPU_PORT_CONTROL_BASE;
   virt_addr= (unsigned long)ioremap(phys_addr,PG_CONTROL_IOMAP_SIZE);
}

/*------  unmap IO addr. -------*/
static void unmap_gpio_reg(void)
{
  iounmap((void *)virt_addr);
}

/*----- st PG11 Multi_Driving Level 3 -------*/
static void set_pg11_level3(void)
{
  volatile unsigned long *pg_conf_reg;
  volatile unsigned long  conf_reg_val;

  pg_conf_reg=(unsigned long *)(virt_addr+PG_DRV0_REG); //--get pg_eint_ctl_reg register value
  conf_reg_val=ioread32(pg_conf_reg);
  //conf_reg_val |= (0b11<<22);
   set_bit(22,&conf_reg_val);  //--!!! set_bit(22,pg_conf_reg) will cause SEGMENTATION FAULT !!!
   set_bit(23,&conf_reg_val);

  iowrite32(conf_reg_val,pg_conf_reg);
}
/*------ set PG11 PULL DOWN ------*/
static void set_pg11_pulld(void)
{
  volatile unsigned long *pg_conf_reg;
  uint32_t  conf_reg_val;

  pg_conf_reg=(unsigned long *)(virt_addr+PG_PULL0_REG); //--get pg_eint_ctl_reg register value
  conf_reg_val=ioread32(pg_conf_reg);
  //conf_reg_val |= (1<<23);
  //conf_reg_val &= ~(1<<22);
  clear_bit(22,(volatile unsigned long *)&conf_reg_val);
  set_bit(23,(volatile unsigned long *)&conf_reg_val);

  iowrite32(conf_reg_val,pg_conf_reg);

}

/*------ clear IRQ pending status -----------*/
static void clear_pg_eint_status(void)
{
  volatile unsigned long *pg_conf_reg;
  uint32_t  conf_reg_val;

  pg_conf_reg=(unsigned long *)(virt_addr+PG_EINT_STATUS_REG); //--get pg_eint_ctl_reg register value
  conf_reg_val=ioread32(pg_conf_reg);
  //---clear PG exteranl interrupt status regsiter----
  conf_reg_val |= (1<<11); //-- write '1' to clear EINT_STATUS
  iowrite32(conf_reg_val,pg_conf_reg);
}

/*------ disable external INT of PG11 -------*/
static void disable_pg11_int(void)
{
  volatile unsigned long *pg_conf_reg;
  uint32_t  conf_reg_val;

  pg_conf_reg=(unsigned long *)(virt_addr+PG_EINT_CTL_REG); //--get pg_eint_ctl_reg register value
  conf_reg_val=ioread32(pg_conf_reg);
  //---disable PG11 int--
  conf_reg_val &= ~(1<<11);
  iowrite32(conf_reg_val,pg_conf_reg);
}

/*------ set PG11 for external interrupt ------*/
static void set_pg11_eint(void)
{
  volatile unsigned long *pg_conf_reg;
  uint32_t  conf_reg_val;

  pg_conf_reg=(unsigned long *)(virt_addr+PG_CFG1_REG); //--get pg_eint_ctl_reg register value
  conf_reg_val=ioread32(pg_conf_reg);
  //---enable gp11 int--
  conf_reg_val |= (0b11<<13);
  conf_reg_val &= ~(1<<12);
  iowrite32(conf_reg_val,pg_conf_reg);
}

/*------ set PG11 IO disable ------*/
static void set_pg11_IOdisable(void)
{
  volatile unsigned long *pg_conf_reg;
  uint32_t  conf_reg_val;

  pg_conf_reg=(unsigned long *)(virt_addr+PG_CFG1_REG); //--get pg_eint_ctl_reg register value
  conf_reg_val=ioread32(pg_conf_reg);
  //---set gp11 IO Disable---
  conf_reg_val |= (0xb111<<12);
  iowrite32(conf_reg_val,pg_conf_reg);
}



/*------ enable external INT of PG11 -------*/
static void enable_pg11_int(void)
{
  volatile unsigned long *pg_conf_reg;
  uint32_t  conf_reg_val;

  pg_conf_reg=(unsigned long *)(virt_addr+PG_EINT_CTL_REG); //--get pg_eint_ctl_reg register value
  conf_reg_val=ioread32(pg_conf_reg);
  //---enable gp11 int--
  conf_reg_val |= (1<<11);
  iowrite32(conf_reg_val,pg_conf_reg);
}

/*------ get PG config register value -------*/
static uint32_t get_pg_config(unsigned long offset)
{
  volatile unsigned long *pg_conf_reg;
  uint32_t  reg_val;

  pg_conf_reg=(unsigned long *)(virt_addr+offset); //--get pg_eint_ctl_reg register value
  reg_val=ioread32(pg_conf_reg);

  return reg_val;
}

/*-------- enable INT in worke queue ---------*/
static void enable_int_wq(struct work_struct *data)
{
   printk(KERN_INFO"Entering work_queue and start msleep....\n");
   gpio_set_value(gpio_beep,1); //--start to beep... 
   msleep(1000);
   gpio_set_value(gpio_beep,0); //--stop beeping ... 
   printk(KERN_INFO"Re-enable irq now....\n");

   clear_pg_eint_status();// clear PG EINT_STATUS
   enable_irq(int_num); // this will both activate PG_EINT11 and PG_EINT_CTL_REG 11 ?????? NOPE!!!
   ////set_pg11_eint();
   enable_pg11_int();

   printk("----- PG_EINT_CTL_REG: 0x%08x -----\n",get_pg_config(PG_EINT_CTL_REG));
}

/*----------  interrupter call handler ----------*/
static irqreturn_t int_handler(int irq, void *dev_id)
{
   //-- to confirm the interrupt ------
   printk("----To Confirm the EINT, PG_EINT_STATUS_REG: 0x%08x -----\n",get_pg_config(PG_EINT_STATUS_REG));

   clear_pg_eint_status();// clear PG EINT_STATUS
   disable_irq_nosync(int_num); //--!!!Must NOT use disable_irq() anyway,it's cause deadloop and will crash the kernel
   //---- !!!! you have to use both of following functions to prevent INT flag change after disable_irq_nosync()
   ////set_pg11_IOdisable();
   disable_pg11_int();

   printk(KERN_INFO "------ gpio_to_irq(PG11) triggered! -----\n");
   printk(KERN_INFO "Disable irq ...\n");
   printk("----- PG_EINT_CTL_REG: 0x%08x -----\n",get_pg_config(PG_EINT_CTL_REG));

   //----- start irq handler low part -------
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

   printk("------ Start init gpio_int -----\n");

   map_gpio_reg();
//   set_pg11_eint(); //--set PG11 for external interrupt


   gpio_direction_output(gpio_beep,0);
   if(gpio_is_valid(gpio_beep) == 0)
   {
	printk("gpio_beep is valid!\n");
	gpio_set_value(gpio_beep,0);
   }
   else
	printk("gpio_beep is NOT valid!\n");

   //------- init PG11 config ------------
   gpio_direction_input(gpio_int); //-set gpio direction
   set_pg11_eint();// set PG11 EINT11
   set_pg11_pulld(); // SET PG11 PULL DOWN
   set_pg11_level3(); // set PG11 Multi-Driving Level3

   if(gpio_is_valid(gpio_int) == 0)
   {
	printk("gpio_int is valid!\n");
	gpio_set_value(gpio_int,0);
   }
   else
	printk("gpio_int is NOT valid!\n");

//   gpio_set_value(gpio_int,0);

   printk("----- PG_CFG1_REG: 0x%08x -----\n",get_pg_config(PG_CFG1_REG));
   printk("----- PG_DRV0_REG: 0x%08x -----\n",get_pg_config(PG_DRV0_REG));
   printk("----- PG_PULL0_REG: 0x%08x -----\n",get_pg_config(PG_PULL0_REG));
   printk("----- PG_EINT_CTL_REG: 0x%08x -----\n",get_pg_config(PG_EINT_CTL_REG));
   printk("----- PG_EINT_CFG1_REG: 0x%08x -----\n",get_pg_config(PG_EINT_CFG1_REG));
   printk("----- PG_CFG1_REG: 0x%08x -----\n",get_pg_config(PG_CFG1_REG));



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

  unmap_gpio_reg();

  printk("------ gpio interrupt test exit -----\n");

}

module_init(gpio_int_init);
module_exit(gpio_int_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux");
MODULE_DESCRIPTION("gpio-int-test");

