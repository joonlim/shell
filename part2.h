#ifndef __PART2FUNCTIONS_H
#define __PART2FUNCTIONS_H

#include "stack.h"
#include <ctype.h>

//function called upon entering of "history into command line"
//maximum reading size of 80 characters
#define MAX_LINE_SIZE 80
#define MAX_FILE_REDIRECTIONS 10
#define MAX_PIPES 10

// Redirection global bools
bool redirect_stdin;
int stdin_redirect_fd;

int otherout_redirect_fd[MAX_FILE_REDIRECTIONS]; 	// contains file descriptors of files we open
int descriptors_to_replace[MAX_FILE_REDIRECTIONS]; 	// contsins descriptors that we should replace
int num_other_redirects;
int pipes_left;
bool piping;
bool is_first_command;
char whole_cmd[MAX_LINE_SIZE];


//if non zero, allow for page down command
int has_pressed_page_up;
int just_paged;
int should_execute_after_page;
//file pointer to look for page up/down presses
FILE*page_search_file_pointer;
//stack for holding history
struct Stack *history_stack;
//stack for holding traversal
struct Stack *cursor_stack;

void parse_pipes(char *cmd, char *all_commands[], int *num_commands);

bool set_stdin_file(char *full_command);
bool set_out_files(char *full_command);

bool file_exists_and_is_not_dir(char *file);
bool file_is_not_dir(char *file);
void reset_redirections();

void change_redirect_descriptors();
void change_piping_descriptors();

int execute_history();
int clear_history();
int page_up();
int page_down();

#endif