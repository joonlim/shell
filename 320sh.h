/**
 * Main header file with all includes and macros
 */

#ifndef __320SH_H
#define __320SH_H

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <stdarg.h>

#include "utils.h"
#include "part2.h"
#include "job_control.h"

/* Macros */
// Colors
#define BRED         	"\x1b[1;31m"
#define CYAN 			"\x1b[36m"
#define YELLOW 			"\x1b[33m"
#define NOCOLOR			"\x1b[0m"
// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 		1024
#define MAX_PROMPT		200
// Write string s to stdout
#define STDOUT(s) 		Write(1, s, strlen(s))
// Write string s to stderr
#define STDERR(s)		Write(2, s, strlen(s))
// History file name
#define HISTORY 		".320sh_history"

/* Global Variables */
const char *promptHead;// = CYAN"[";
const char *promptTail;// = "]"CYAN NOCOLOR" "BRED"320sh> "NOCOLOR;
char *pwd;
// if program is run with -d, display debugging info to stderr.
int debug;
// if the user pressed backspace, this will be 1 and the prompt will not be reprinted
int has_backspaced;
// envp for this shell
char **envp320;
int envp_size;
char **new_var_mapping; // use this to store new variable mappings whenever set is called
int new_map_size;
//flag to see if this is a background process - if 0 false else true;
int is_background_process;
// $HOME/.320sh_history
FILE *ofp;

//this pointer will be used to store any additional text that might result from backspace or page up/down
char old_text[MAX_INPUT];

bool pressed_up_or_down;

//moved from main
char cmd[MAX_INPUT];      // array to hold input
char *last_line;
int last_line_poz;                       // used to store place in last_line array
char *startPtr;
    
//passed in at program start
int shell_argc;
char ** shell_argv;
char ** shell_envp;

/**
 * Function that will be called by atexit().
 */
void house_keeping();

//initializes all variables needed for program exec
void initialize_variables(int argc, char ** argv, char **envp);
void main_loop();


#endif
