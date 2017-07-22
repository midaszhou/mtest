#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL v2");

static __init int lirc_init(void)
{

    printk(KERN_INFO " --------- lirc init -------\n");
    return 0;

}

static __exit void lirc_exit(void)
{

    printk(KERN_INFO " --------- lirc exit -------\n");
}

module_init(lirc_init)
module_exit(lirc_exit)


