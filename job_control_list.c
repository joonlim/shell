//This linked list is used to hold the jobs in the shell
#include "job_control_list.h"

void print_job_at_position(int position, struct job_list* list)
{
	struct job_node* data = get_job_at_position(position, list);
	if(data == NULL)
	{
		return;
	}
	else
	{
		STDOUT("GOT Job at position\n");
	}
}

//initialize list to NULL
struct job_list* init_job_list()
{
	struct job_list* linked_list = (struct job_list *)malloc(sizeof(struct job_list));
	
	linked_list->head = NULL;
	linked_list->tail = NULL;
	linked_list->pointer = NULL;
	linked_list->size = 0;

	return linked_list;
}

//given an index, remove the right item from the list (0 indexed)
struct job_node* remove_from_job_list(int poz, struct job_list *linked_list)
{
	if(poz > linked_list->size)
	{
		STDOUT("ERROR INSERTING TO JOB_LIST, position is greater than size\n");
		return NULL;
	}

	struct job_node* ret_val = NULL;

	//if we're removing the head, make the next item the head
	if(poz == 0)
	{
		struct job_node *old_head = linked_list->head;
		struct job_node *new_head = old_head->next_link;

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

			return old_head;
		}

		free(old_head);
	}
	//if we're removing the tail, do this
	else if(poz == linked_list->size - 1)
	{
		struct job_node *old_tail = linked_list->tail;
		struct job_node *new_tail = old_tail->previous_link;

		//note, the list cant really be empty after deletion since that condition is checked in the previous if block
		linked_list->tail = new_tail;
		new_tail->next_link = NULL;

		ret_val = old_tail;
		free(old_tail);
	}
	//deletion is occuring inside the linked list but it is not the head or tail
	else
	{
		struct job_node *previous_link = linked_list->head, *deletion_link, *next_link;
		int i = 0;

		while(i < poz - 1)
			previous_link = previous_link->next_link;
		
		deletion_link = previous_link->next_link;
		next_link = previous_link->next_link->next_link;

		//set the links of previous and next
		previous_link->next_link = next_link;
		next_link->previous_link = previous_link;

		//free the link to be deleted
		ret_val = deletion_link;
		free(deletion_link);
	}

	linked_list->size--;
	return ret_val;
}

//insert the given item into the list (0 indexed)
/*char* insert_into_job_list(char *name, int poz, struct job_list *linked_list)
{
	if(poz > linked_list->size)
	{
		STDOUT("ERROR INSERTING TO LIST, position is greater than size\n");
		return NULL;
	}
	
	//make the node
	struct job_node *node = (struct job_node *) malloc(sizeof(struct job_node));
	init_job_node(node);
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
}*/

//get job in position of list (list is zero indexed)
struct job_node* get_job_at_position(int poz, struct job_list* linked_list)
{
	if(linked_list->size == 0)
	{
		STDOUT("ERROR: job_list does not have any members\n");
		return NULL;
	}
	else if(poz >= linked_list->size)
	{
		STDOUT("ERROR: job_list index out of bounds\n");
		return NULL;

	}

	//if the position is zero, return data at the head
	if(poz == 0)
		return linked_list->head;
	//if the position is the tial, return the data at the tail
	else if(poz == linked_list->size - 1)
		return linked_list->tail;
	//otherwise go to the position in the list and return the item data
	else
	{
		int i = 0;
		struct job_node *node = linked_list->head;
		while(i < poz)
		{
			node = node->next_link;
			i++;
		}
		return node;
	}

	//just to silence warning
	return NULL;
}

struct job_node* get_node_by_id(int process_id, struct job_list* linked_list)
{
	struct job_node *node = linked_list->head;
	while(1)
	{
		if(node->process_id == process_id)
			return node;

		node = node->next_link;

		//if the node becomes null return NULL since we did not find item
//		STDOUT("Did not find job\n");
		return NULL;
	}
}

struct job_node* get_node_by_job_id(int job_id, struct job_list* linked_list)
{
	struct job_node *node = linked_list->head;
	while(1)
	{
		if(node->job_id == job_id)
			return node;

		node = node->next_link;

		//if the node becomes null return NULL since we did not find item
	//	STDOUT("Did not find job\n");
		if(node == NULL)
		{
			STDOUT("job id was ");

			char conversion_buff[100];
			strcpy(conversion_buff, "");	//clear buffer
			sprintf(conversion_buff, "%d", job_id);
			STDOUT(conversion_buff);
			STDOUT("\n");

			return NULL;
		}
	}
}

int add_job_to_list(char* name, char* status, int process_id, int job_number, int job_id, int is_foreground, struct job_list *linked_list)
{
	//make the node
	struct job_node *node = (struct job_node *) malloc(sizeof(struct job_node));
	init_job_node(node);

	//set all data of node
	strcpy(node->name , name);
	strcpy(node->status, status);
	node->process_id = process_id;
	node->job_number = job_number;
	node->job_id = job_id;
	node->is_foreground = is_foreground;
		
	//append to end of list
	struct job_node *tail = linked_list->tail;

	//if the tail is null, make the new item both head and tail and return
	if(tail == NULL)
	{
		linked_list->head = node;
		linked_list->tail = node;
		linked_list->size++;

		return EXIT_SUCCESS;
	}

	tail->next_link = node;
	node->previous_link = tail;

	linked_list->tail = node;			

	return EXIT_SUCCESS;
}

int update_status(int process_id, char* new_status, struct job_list *list)
{
	struct job_node *node = get_node_by_id(process_id, list);
	if(node == NULL)
		return EXIT_FAILURE;

	strcpy(node->status , new_status);

	return EXIT_SUCCESS;
}

int update_name(int process_id, char* new_name, struct job_list *list)
{
	struct job_node* node = get_node_by_id(process_id, list);
	if(node == NULL)
		return EXIT_FAILURE;

	strcpy(node->name , new_name);

	return EXIT_SUCCESS;
}

int update_job_id(int process_id, int new_job_id, struct job_list *list)
{
	struct job_node *node  = get_node_by_id(process_id, list);
	if(node == NULL)
		return EXIT_FAILURE;

	node->job_id = new_job_id;

	return EXIT_SUCCESS;
}

int update_foreground(int process_id, int new_is_foreground, struct job_list *list)
{
	struct job_node *node = get_node_by_id(process_id, list);
	if(node == NULL)
		return EXIT_FAILURE;

	node->is_foreground = new_is_foreground;

	return EXIT_SUCCESS;
}

int update_job_number(int process_id, int new_job_number, struct job_list *list)
{
	struct job_node *node = get_node_by_id(process_id, list);
	if(node == NULL)
		return EXIT_FAILURE;

	node->job_number = new_job_number;

	return EXIT_SUCCESS;
}

int retire_all_processes(struct job_list* linked_list)
{
	if(linked_list->size == 0)
		return EXIT_FAILURE;

	linked_list->size = 0;

	struct job_node *ptr = linked_list->head;
	struct job_node *old_ptr;
	//while the re are jobs, kill them all
	while(ptr != NULL)
	{
		old_ptr = ptr;
		ptr = ptr->next_link;

		free(old_ptr);
	}

	return EXIT_SUCCESS;
}


//given an index, remove the right item from the list (0 indexed)
int retire_process(int job_id, struct job_list *linked_list)
{
	//if there are no items, return null
	if(linked_list->size == 0)
		return EXIT_FAILURE;

	//get process node assosc with ID
	struct job_node *node = get_node_by_job_id(job_id, linked_list);
	//if the process doesnt exit return null
	if(node == NULL)
		return EXIT_FAILURE;

	//if we're removing the head, make the next item the head
	if(node->previous_link == NULL)
	{
		struct job_node *old_head = linked_list->head;
		struct job_node *new_head = old_head->next_link;

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

			return EXIT_SUCCESS;
		}

		free(old_head);
	}
	//if we're removing the tail, do this
	else if(node->next_link == NULL)
	{
		struct job_node *old_tail = linked_list->tail;
		struct job_node *new_tail = old_tail->previous_link;

		//note, the list cant really be empty after deletion since that condition is checked in the previous if block
		linked_list->tail = new_tail;
		new_tail->next_link = NULL;

		free(old_tail);
	}
	//deletion is occuring inside the linked list but it is not the head or tail
	else
	{
		//we are storing current process in node
		struct job_node *previous_link = node->previous_link, *next_link = node->next_link;
		
		//set the links of previous and next
		previous_link->next_link = node->next_link;
		next_link->previous_link = node->previous_link;

		//free the link to be deleted
		free(node);
	}

	linked_list->size--;
	return EXIT_SUCCESS;
}

int* return_all_job_ids(struct job_list* list)
{
	int *job_id_array = (int *)malloc(sizeof(int) * MAX_LIST_SIZE);
	int i = 0;
	struct job_node* node = list->head;
	while(node != NULL)
	{
		//char conversion_buff[MAX_INT_CONVERSION_LENGTH];
        //print the job id 
        //STDOUT("job beingadded is: ");
        //sprintf(conversion_buff, "%d", node->job_id);
        //STDOUT(conversion_buff);
        //STDOUT("\n");

		job_id_array[i] = node->job_id;
		i++;
		node = node->next_link;
	}

	return job_id_array;
}

int* return_all_proc_ids(struct job_list* list)
{
	int *job_id_array = (int *)malloc(sizeof(int) * MAX_LIST_SIZE);
	int i = 0;
	struct job_node* node = list->head;
	while(node != NULL)
	{
		job_id_array[i] = node->process_id;
		i++;
		node = node->next_link;
	}

	return job_id_array;
}

int get_num_jobs(struct job_list* list)
{
	//int *job_id_array = (int *)malloc(sizeof(int) * MAX_LIST_SIZE);
	int i = 0;
	struct job_node* node = list->head;
	while(node != NULL)
	{
		i++;
		node = node->next_link;
	}

	return i;
}

void init_job_node(struct job_node *node)
{
	//set links of node
	node->previous_link = NULL;
	node->next_link = NULL;

	//set data of node - all null for now
	node->name = (char*) malloc(100);
	node->status = (char *) malloc(100);
	node->is_foreground = -1;
	node->process_id = -1;
	node->job_id = -1;
	node->job_number = -1;
}