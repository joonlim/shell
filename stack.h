//This is a linked list implementation of a stack, supporting operations push, pop, and peek.
#ifndef LINKED_LIST_GUARD
#define LINKED_LIST_GUARD 0
#include "linkedlist.h"
#include <string.h>
#include "utils.h"

struct Stack
{
	struct List *list;
	int size;
};

struct Stack* init_stack();
char *push(char *data, struct Stack* stack);
char *pop(struct Stack* stack);
char *peek(struct Stack* stack);

#endif
