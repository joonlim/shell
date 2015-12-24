#ifndef __JOBCONTROLLLIST_H
#define __JOBCONTROLLLIST_H

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

#define MAX_LIST_SIZE 1024

struct job_node
{
	//the name of the running process
	char* name;
	//status is either RUNNING or SUSPENDED
	char* status;
	//a value of 1 means it is, else it is not
	int is_foreground;
	int process_id;
	int job_id;
	int job_number;
	struct job_node *previous_link;
	struct job_node *next_link;
};

struct job_list
{
	struct job_node *head;
	struct job_node *tail;
	struct job_node *pointer;
	int size;
};

/*
	THESE ARE GENERIC LINKED LIST METHODS
*/
//char* insert_into_job_list(char *data, int poz, struct job_list *list);
//given an index, remove the right item from the list (0 indexed)
struct job_node* remove_from_job_list(int poz, struct job_list *list);
struct job_node* get_job_at_position(int position, struct job_list *list);
void print_job_at_position(int position, struct job_list *list);


/*
	THESE METHODS ARE NEEDED FOR JOB CONTROL
*/
struct job_list* init_job_list();
void init_job_node(struct job_node *node);
//append job to end of list - MUST HAVE VALID JOB ID
int add_job_to_list(char* name, char* status, int process_id, int job_number, int job_id, int is_foreground, struct job_list *linked_list);
//given a process_id, get the node (pid)
struct job_node* get_node_by_id(int process_id, struct job_list* linked_list);
//get by job id node
struct job_node* get_node_by_job_id(int job_id, struct job_list* linked_list);

//remove a project from the list
int retire_process(int process_id, struct job_list *linked_list);
int retire_all_processes(struct job_list* linked_list);


//UPDATE METHODS
int update_status(int process_id, char* new_status, struct job_list *list);
int update_foreground(int process_id, int new_is_foreground, struct job_list *list);
int update_name(int process_id, char* new_name, struct job_list *list);
int update_job_id(int process_id, int new_job_id, struct job_list *list);
int update_job_number(int process_id, int new_job_number, struct job_list *list);

//methods to get pid and jids
int* return_all_proc_ids(struct job_list* list);
int* return_all_job_ids(struct job_list* list);
int get_num_jobs(struct job_list* list);


#endif