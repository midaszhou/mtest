#include <stdio.h>
#include <stdlib.h>
//#include <linux/types.h>
#include "list.h"

struct test_list{
    int data;
    struct list_head list;
};

int main()

{

    LIST_HEAD(boy);  //define and initialize a doubly linked of boy

    int i;
    struct test_list *ptr[10];
    struct list_head *tmp = NULL;
    struct test_list *node;

    for(i = 0; i < 10; i++)
    {
        ptr[i] = (struct test_list *)malloc(sizeof(struct test_list));
        if(!ptr[i])
            printf("error");

        ptr[i]->data = i;
    }

    for(i = 0; i < 10; i++)
    {
        list_add_tail(&ptr[i]->list, &boy);
    }

    printf("Traverse link:\n");

    list_for_each(tmp, &boy)       //boy为链表的头节
    {                                          // tmp为list_head类型的指针用来暂时存放节点
        node = list_entry(tmp, struct test_list, list);  //node为包含list_head的结构这里类    
        printf("%d ", node->data);
    }

    printf("\n");

    return 0;

}
