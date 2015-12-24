// This is a simple linked list data structure written with the C language
#include "linkedlist.h"

void print_item_at_position(int position, struct List* list)
{
	char* data;

	data = get_item_at_position(position, list);
	if(data == NULL)
	{
		return;
	}
}

//initialize list to NULL
struct List* init_list()
{
	struct List* linked_list = (struct List *)malloc(sizeof(struct List));
	
	linked_list->head = NULL;
	linked_list->tail = NULL;
	linked_list->pointer = NULL;
	linked_list->size = 0;

	return linked_list;
}

//given an index, remove the right item from the list (0 indexed)
char* remove_from_list(int poz, struct List *linked_list)
{
	if(poz > linked_list->size)
	{
		STDOUT("ERROR INSERTING TO LIST, position is greater than size\n");
		return NULL;
	}

	char* ret_val = NULL;

	//if we're removing the head, make the next item the head
	if(poz == 0)
	{
		struct Node *old_head = linked_list->head;
		struct Node *new_head = old_head->next_link;

		if(new_head != NULL)
		{
			linked_list->head = new_head;
			new_head->previous_link = NULL;
		}
		//the list is empty now
		else
		{
			linked_list->head = NULL;
			linked_list->tail = NULL;
			linked_list->pointer = NULL;
			linked_list->size = 0;

			return old_head->data;
		}

		free(old_head);
	}
	//if we're removing the tail, do this
	else if(poz == linked_list->size - 1)
	{
		struct Node *old_tail = linked_list->tail;
		struct Node *new_tail = old_tail->previous_link;

		//note, the list cant really be empty after deletion since that condition is checked in the previous if block
		linked_list->tail = new_tail;
		new_tail->next_link = NULL;

		ret_val = old_tail->data;
		free(old_tail);
	}
	//deletion is occuring inside the linked list but it is not the head or tail
	else
	{
		struct Node *previous_link = linked_list->head, *deletion_link, *next_link;
		int i = 0;

		while(i < poz - 1)
			previous_link = previous_link->next_link;
		
		deletion_link = previous_link->next_link;
		next_link = previous_link->next_link->next_link;

		//set the links of previous and next
		previous_link->next_link = next_link;
		next_link->previous_link = previous_link;

		//free the link to be deleted
		ret_val = deletion_link->data;
		free(deletion_link);
	}

	linked_list->size--;
	return ret_val;
}

//insert the given item into the list (0 indexed)
char* insert_into_list(char *data, int poz, struct List *linked_list)
{
	if(poz > linked_list->size)
	{
		STDOUT("ERROR INSERTING TO LIST, position is greater than size\n");
		return NULL;
	}
	
	//make the node
	struct Node *node = (struct Node *) malloc(sizeof(struct Node));
	init_node(node, data);
	//node->data = data;

	//if there are no items in the list (head is NULL) new node is head and tail
	if(linked_list->head == NULL)
	{
		node->next_link = NULL;
		node->previous_link = NULL;

		linked_list->head = node;
		linked_list->tail = node;

		linked_list->pointer = node;
	}

	//if the position is the last in the list
	else if(poz == linked_list->size)
	{
		struct Node *tail = linked_list->tail;
		tail->next_link = node;
		node->previous_link = tail;

		linked_list->tail = node;
	}
	//if you are inserting into the head of the list
	else if(poz == 0)
	{
		struct Node *head = linked_list->head;
		
		head->previous_link = node;
		node->next_link = head;

		linked_list->head = node;
	}
	//else insert it into the middle of the list
	else
	{
		struct Node *previous_link = linked_list->head;
		int i = 0;

		while(i < poz - 1)
			previous_link = previous_link->next_link;
		
		struct Node *next_link = previous_link->next_link;

		previous_link->next_link = node;
		node->previous_link = previous_link;

		next_link->previous_link = node;
		node->next_link = next_link;
	}

	linked_list->size++;

	return "EXIT_SUCCESS";
}

//get item in position of list (list is zero indexed)
char* get_item_at_position(int poz, struct List* linked_list)
{
	if(linked_list->size == 0)
	{
		STDOUT("ERROR: Linked list does not have any members\n");
		return NULL;
	}
	else if(poz >= linked_list->size)
	{
		STDOUT("ERROR: Linked list index out of bounds\n");
		return NULL;

	}

	//if the position is zero, return data at the head
	if(poz == 0)
		return linked_list->head->data;
	//if the position is the tial, return the data at the tail
	else if(poz == linked_list->size - 1)
		return linked_list->tail->data;
	//otherwise go to the position in the list and return the item data
	else
	{
		int i = 0;
		struct Node *node = linked_list->head;
		while(i < poz)
		{
			node = node->next_link;
			i++;
		}
		return node->data;
	}
}

void init_node(struct Node *node, char* command)
{
	node->data = (char*) malloc(100);
	strcpy(node->data, command);

	node->previous_link = NULL;
	node->next_link = NULL;
}