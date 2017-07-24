//--- try to force to remove kernel moduel ----
//--- get module completion pointer by  'cat /proc/kallsyms | grep MODULE_NAME | grep this_module'

#define CONFIG_MODULE_UNLOAD
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/completion.h>
#include <asm/local.h>
//#include <linux/moduleparam.h>
//#include <linux/param.h>


void force(void)
{
}

static __init int rm_init(void)
{
  complete((struct completion *)0xbf0f7040);
/*
struct module *mod = (struct module*)0xbf0f7040;
        int i;
        int o=0;
        mod->state = MODULE_STATE_LIVE; //为了卸载能进行下去，也就是避开情况1，将模块的状态改变为LIVE
        mod->exit = force;    //由于是模块的exit导致了无法返回，则替换mod的exit。再次调用rmmod的时候会调用到sys_delete_module，最后会调用 exit回调函数，新的exit当然不会阻塞，成功返回，进而可以free掉module
        for (i = 0; i < NR_CPUS; i++)
        { //将引用计数归0
                mod->ref[i].count = *(local_t *)&o;
        }
*/
    return 0;
}

static __exit void rm_exit(void)
{
}

module_init(rm_init);
module_exit(rm_exit);

MODULE_LICENSE("GPL");
 
