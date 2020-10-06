#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	int val;
	char expr[32];
	char swi;
	

	/* TODO: Add more members if necessary */


} WP;

WP* new_wp();
void free_wp(int);
bool check_wp();
void print_wp();
#endif
