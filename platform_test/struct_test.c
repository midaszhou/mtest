#include <stdio.h>
#include <string.h>
#include  <stdlib.h>

struct person_id;
int act_print(struct person_id *p);


struct  person_id {
char name[10];
int age;
int (*action)(struct person_id *);
};


int act_print(struct person_id *p)
{
  if( p && (p->action))
    printf(" ---- act print: age=%d -----\n",p->age);
}


int main(void)
{

struct person_id *Mike=(struct person_id*)malloc(sizeof(struct person_id));

struct  person_id jack={
	 .name="Jack",
	 .age=24,
	 .action=act_print,
  };

jack.action(&jack);

(*Mike).age=12;
Mike->action = act_print;
strcpy(Mike->name,"Mike");
Mike->action(Mike);


 return 0;

}
