//--- try to force to remove kernel moduel ----
//--- get module completion pointer by  'cat /proc/kallsyms | grep MODULE_NAME | grep this_module'

#define CONFIG_MODULE_UNLOAD
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/completion.h>
#include <asm/local.h>
//#include <linux/moduleparam.h>
//#include <linux/param.h>

static __init int rm_init(void)
{
        struct list_head *modules=(struct list_head *)0xbf0f7080;
        struct module *mod=0;
        struct module *list_mod;
        int i;
        int zero=0;

        list_for_each_entry(list_mod,modules,list){
                if(strcmp(list_mod->name,"kobj") == 0)
                  mod=list_mod;
        }

        mod->state=MODULE_STATE_LIVE;

/*
        for (i = 0; i < NR_CPUS; i++){
                mod->ref[i].count=*(local_t *)&zero;
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
 
