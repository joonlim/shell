#include "stack.h"

struct Stack* init_stack()
{
	struct Stack *stack = (struct Stack *)malloc(sizeof(struct Stack));

	struct List* list = init_list();
	stack->list = list;
	stack->size = 0; 

	return stack;
}


char* push(char *data, struct Stack* stack)
{
	if(strcmp(insert_into_list(data, stack->size, stack->list), "EXIT_SUCCESS") != 0)
	{
		return NULL;
	}
	else
	{
		stack->size = stack->list->size;
		return data;
	}
}

char* pop(struct Stack* stack)
{
	char *data = NULL;
	if(stack->size == 0)
	{
		return NULL;
	}
	else if((data = remove_from_list(stack->size - 1, stack->list)) == NULL)
	{
		return data;
	}
	else
	{
		stack->size = stack->list->size;
		return data;
	}
}

char* peek(struct Stack* stack)
{
	char *data = NULL;
	if(stack->size == 0)
	{
		return NULL;
	}
	if((data = get_item_at_position(stack->size - 1, stack->list)) == NULL)
	{
		return data;
	}
	else
		return data;	
}
