#ifndef __STACK_H
#define __STACK_H

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

struct Node
{
	char* data;
	struct Node *previous_link;
	struct Node *next_link;
};

struct List
{
	struct Node *head;
	struct Node *tail;
	struct Node *pointer;
	int size;
};

char* insert_into_list(char *data, int poz, struct List *list);
//given an index, remove the right item from the list (0 indexed)
char* remove_from_list(int poz, struct List *list);
char* get_item_at_position(int position, struct List *list);
void print_item_at_position(int position, struct List *list);
struct List* init_list();
void init_node(struct Node *node, char *data);

#endif