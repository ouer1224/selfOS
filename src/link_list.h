
#ifndef __LINK_LIST_H__
#define __LINK_LIST_H__

#include "os_def.h"



struct __link_list{
	struct __link_list  *pre;
	struct __link_list  *next;
};



#define __offsetof(type,member)	(size_t)(&(((type *)0)->member))

#define container_of(pr_list,type,member)	__container_of(pr_list,__offsetof(type,member))



int list_add_behind(struct __link_list *new_list,struct __link_list *current);
void * __container_of(void *cur,int offset);


int list_del(struct __link_list *current);




#endif

