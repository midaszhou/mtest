#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/list.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Xie");
MODULE_DESCRIPTION("List Module");
MODULE_ALIAS("List module");

struct student
{
    char name[100];
    int num;
    struct list_head list;
};

struct student *pstudent;
struct student *tmp_student;
struct list_head student_list;
struct list_head *pos,*ltmp;

int __init mylist_init(void)
{
	int i = 0;

        printk("------ start list_test -----\n");

	INIT_LIST_HEAD(&student_list);

	pstudent = kmalloc(sizeof(struct student)*5,GFP_KERNEL);
	memset(pstudent,0,sizeof(struct student)*5);

	for(i=0;i<5;i++)
	{
	        sprintf(pstudent[i].name,"Student%d",i+1);
		pstudent[i].num = i+1;
		list_add( &(pstudent[i].list), &student_list);
	}


	list_for_each(pos,&student_list)
	{
		tmp_student = list_entry(pos,struct student,list);
		printk("<0>student %d name: %s\n",tmp_student->num,tmp_student->name);
	}

	return 0;
}


void __exit mylist_exit(void)
{
	int i ;

        printk("exiting list_test... \n");

        list_for_each_safe(pos,ltmp,&student_list)  // !!!! when delete list you must use list_for each_safe()
	{
           tmp_student=list_entry(pos,struct student,list);
	   printk("del student[%d] from student list...\n",tmp_student->num);
	   list_del(&(tmp_student->list));
	}

/*
	for(i=0;i<5;i++)
	{
		list_del(&(pstudent[i].list));

	}
*/

	kfree(pstudent);
        printk("------ list_test exits -----\n");
}

module_init(mylist_init);
module_exit(mylist_exit);

