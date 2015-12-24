#ifndef __PART1FUNCTIONS_H
#define __PART1FUNCTIONS_H

#include "320sh.h"
#include "job_control.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>

//This will hold the previous directory the user was in
//and is needed for cd -
#define MAX_DIRECTORY_LENGTH 100

// Maximum number of indivial strings we can allocate for each command when
// we need to replace variable names with their bindings.
#define MAX_VAR 200

char prev_directory[MAX_DIRECTORY_LENGTH];
char curr_directory[MAX_DIRECTORY_LENGTH];

//a value of 0 means that we are reading a script, nonzero means we are not
int is_reading_script;


void init_directories();

//value of 0 means we are not in script, 1 means we are in script
int in_scipt;
//we initialy are reading the first line, should be initialized to 1
int reading_first_line;
    		

void trim_and_move_redundant_spaces(char *old_text);

/**
 * Given a list of options, determine if any valid arguments are given.
 * This function causes the program to terminate if any invalid arguments are
 * given.
 * 
 * @param argc [input] number of arguments passed into program execution
 * @param argv [input] array of arguments passed into program execution
 */
void check_options(int argc, char** argv);

/**
 * Given a list of environment variables taken from main(), create a dynamic 
 * copy of them and sorts the array. This function returns a pointer to the
 * region and also return the size of the array in envp_size.
 * 
 * @param  envp      [input]  environment variables taken from main
 * 
 * @return           pointer to allocated dynamic array envp.
 */
char ** initialize_envp(char **envp);

/**
 * Given a special character, decide which special command to run.
 *
 * List of special commands:
 *
 * Ctrl+C 			terminate program
 * Backspace 		erase last character
 * Tab 				auto-complete command
 * Up 				show previous command
 * Down 			show next command
 * Right 			move cursor right
 * Left 			move cursor left
 * 
 * @param  c [input] cursor to check
 * @return   indicates whether the for loop reading user input shoudl end.
 */
bool execute_special_command(char *cursor, char *startPtr, char *last_line_arr, int last_line_poz);

/**
 * Executes a command given in the form of a string.
 * The string cmd will be delimited by (\s)*.
 * The first word in the string will be the command.
 * The follow words will be arguments to the command.
 * 
 * @param cmd  [input] a string representing user input.
 * @param envp [input] environment variable list
 * @param called_recursively - a value of 1 indicates this was called from another call of execute command
 */
void execute_command(char *cmd, char *envp[], int called_recursively);

/**
 * Given one string of commands representing user input, return a space 
 * delimited array of argument tokens. This function treats multiple spaces as
 * one space.
 * 
 * @param  cmd  [input] a string representing user input.
 * @param  argc [output] the number of arguments in the command.
 * @return      an array of arguments (*argv[])
 */
char **initialize_argv(char *cmd, int *argc);

/**
 * Given an array of arguments, replace any substring $WORD with a binding
 * found in the global variable envp320. If no binding is found, replace with
 * the empty string "". If the variable is $?, then replace with the previous
 * exit code.
 *
 * Use variable_buffer to allocate space for the expanded variables and the
 * string that contains them.
 * 
 * @param argc 			  [input]  Number of indices of argv
 * @param argv 			  [input]  List of arguments
 * @param variable_buffer [input]  String buffer that will contain replaced 
 *                        		   arguments
 * @param indices_used 	  [output] The number of indices of variable_buffer we
 *                        		   use. This is updated so we know how many
 *                        		   indices of variable_buffer to free.
 */
void replace_variables(int argc, char **argv, char *variable_buffer[MAX_VAR], int *indices_used);

/**
 * Given a variable name, search for its mapping in the global variable
 * envp320, which contains a list of strings that contain mappings in the form
 * "NAME=BINDING" and set binding to BINDING.
 * 
 * If the name exists, but the there is no binding after the equals sign, do
 * not set binding. 
 *
 * If the name does not exist, do not set binding.
 * 
 * @param name     [input]  variable name
 * @param binding  [output] variable's binding
 */
char *get_binding(const char *name);

/**
 * Append cmd_string in $HOME/.320sh_history on a new line.
 * 
 * @param cmd_string [input] Command to save to history.
 */
void save_to_history(char *cmd_string);

/**
 * Check if the command in argv[0] is in the path and execute it if it is.
 * If argv[0] is a path, this function first checks to see if that file exists,
 * and then executes it. If not, it checks every path in $PATH to see if it
 * exists.
 *
 * @param  argv [input] array of arguments + NULL as last element
 * @param  envp [input] environment variable list
 * @return      true if this succeeds in finding the command, else false.
 */
bool execute_binary(char *argv[], char *envp[], int was_recursive_call, int argc, int job_id);

/**
 * Check if this file exists.
 * 
 * @param  file [input] a path to a file
 * @return         true if the file is found, else false
 */
bool file_exists(char *file);

/**
 * Use fork() to create a child process and execute a command with args.
 * 
 * @precondition: The command file must exist!
 * 
 * @param  command [input] the path to a command
 * @param  argv    [input] array of arguments + NULL as last element
 * @param  envp    [input] environment variable list
 */
void execute_in_child_process(char *command, char *argv[], char *envp[], int job_id, int called_from_script);

//Nonblocking version of top
void execute_in_child_process_nonblocking(char *command, char *argv[], char *envp[], int job_id, int called_from_script);


#endif