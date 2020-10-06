#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
		wp_pool[i].swi='F';
		wp_pool[i].val=0;
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}
WP* new_wp(){
	WP *p,*q;
	q=free_;
	free_=free_->next;
	free_->next=NULL;
	if(head==NULL){
		head=q; p=head;
	}
	else{
		p=head;
		while(p->next!=NULL) p=p->next;
		p->next=q;
	}
	q->swi='O';
	return q;
}

void free_wp(int num){
	WP *p,*q;
	q=NULL;
	p=head;
	while(p!=NULL&&p->NO!=num){
		q=p;p=p->next;
	}
	if(p==NULL) Assert(1,"Wrong Input!");
	if(q==NULL){
		head=head->next;
		p->val=0;
		p->expr[0]='\0';
		p->swi='F';
		p->next=free_;
		free_=p;
	}
	else{
		q->next=p->next;
		p->val=0;
		p->expr[0]='\0';
		p->swi='F';
		p->next=free_;
		free_=p;
	}
	
}

bool check_wp(){
	WP* p;
	bool suc;
	p=head;
	int temp;
	if(head==NULL) return true;
	while(p){
		temp=expr(p->expr,&suc);
		if(temp!=p->val) return false;
		p=p->next;
	}
	return true;
}

void print_wp(){
	int i;
	printf("NO\tswitch\tvalue\t\twhat\n");
	for(i=0;i<NR_WP;i++){
		printf("%4d\t%c\t0x%x\t\t%s\n",wp_pool[i].NO,wp_pool[i].swi,wp_pool[i].val,wp_pool[i].expr);
	}
}
		
/* TODO: Implement the functionality of watchpoint */


