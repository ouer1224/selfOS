

#include <stdint.h>

#include "link_list.h"

#include "null.h"


//#define numofbuffer(a)		(sizeof(a)/sizeof(a[0]))
#define sizeof_V(a) 			((size_t)(&a+1)-(size_t)(&a))
#define numofbuffer(a)		(sizeof_V(a)/(sizeof_V(a[0])))	


#define link_list struct __link_list


#define init_list_for_type(array)	__init_list_for_type(&array[0].list,numofbuffer(array),sizeof_V(array[0]))

static struct __link_list *shead=NULL,*stail=NULL;


int init_link_list(link_list *list)
{
	if(list==NULL)
	{
		return -1;
	}	
	
	list->pre=list;
	list->next=list;
	
	return 0;
}

int __list_add(link_list *new_list,link_list *pre,link_list *next)
{
	if((new_list==NULL)||(pre==NULL)||(next==NULL))
	{
		return -1;		
	}
	
	new_list->next=next;
	next->pre=new_list;
	
	new_list->pre=pre;
	pre->next=new_list;	

	return 0;	
}
/*
��current�ڵ�֮������һ���µ�new_list�ڵ� 
*/
int list_add_behind(link_list *new_list,link_list *current)
{
	return __list_add(new_list,current,current->next);		
}

/*
��current�ڵ�֮ǰ����һ���µ�new_list�ڵ� 
Ҳ���ǽ� 
*/
int list_add_before(link_list *new_list,link_list *current)
{
	return __list_add(new_list,current->pre,current);		
}

#if 1
/*
����ֵΪ0ʱ,��ʾ��,����ֵΪ1ʱ��ʾ��,����ֵΪ-1ʱ,��ʾ�ǿ�,���� 
*/
int list_empty_or_full(link_list *list) 
{
	if(list->pre==NULL)
	{
		return 0;
	}
	if(list->next==NULL)
	{
		return 1;
	}
	
	return -1;	
}
#endif 

link_list * __init_list_for_type(link_list *head,int num,int size)
{
	link_list *cur=NULL,*next=NULL;
	int i=0;
	
	init_link_list(head);

	cur=head;
	for(i=1;i<num;i++)
	{
		next=(void *)((size_t)((void*)head)+size*i);
		
		list_add_behind(next,cur);

		cur=next;		
	}
	
	return head;
}

void * __container_of(void *cur,int offset)
{
	void *pr=NULL;
	
	pr=(void *)((size_t)cur-offset);

	return pr;	
}

int link_full(void)//��ѭ��������,headָ��ָ���ŵ�ǰ����д�ĿյĿռ�,���head�Ǳ�д,��++.��headָ�����һ�����ÿռ�ʱ��Ϊ�� 
{
	return (shead->next==stail);
}
int link_empty(void)
{
	return (stail==shead);
}


#if 0
int put_dat_into_link(link_list *dat,int size)
{
	void *pr_from;
	void *pr_to;
	
	if(link_full()==1)
	{
		return 0;
	}
	size=size-sizeof(link_list);

	pr_from=container_of((void *)dat,freeze_dat_Ah,list);
	pr_to=container_of((void *)shead,freeze_dat_Ah,list);
	memcpy(pr_to,pr_from,size);
	
	shead=shead->next;
	
	return 1;
}

int get_dat_from_link(link_list *dat,int size)
{
	void *pr_from;
	void *pr_to;
	
	if(link_empty()==1)
	{
		return 0;
	}	
	size=size-sizeof(link_list);
	pr_to=container_of((void *)dat,freeze_dat_Ah,list);
	pr_from=container_of((void *)stail,freeze_dat_Ah,list);
	memcpy(pr_to,pr_from,size);
	
	stail=stail->next;
	
	return 1;
}

#endif



