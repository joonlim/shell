#ifndef __JOBCONTROL_H
#define __JOBCONTROL_H
#include "utils.h"
#include <sys/wait.h>
#include <signal.h>
#include "job_control_list.h"

//max int conversion buffer size
#define MAX_INT_CONVERSION_LENGTH 10

//File will handle all job control functionality of 320sh
//next number to assign for PID
int next_pid_number;
//next number to assign for JID
int next_jid_number;
//the list used for job control
struct job_list* list_of_jobs;

//need this to do proper variable initialization at startup
int init_job_control_funcs();
//adds job to job list
//returns job id number
int add_job(char *job_name, int job_id);
//removes job from job list
void kill_job(char *job_name, int pid);
//kill all foreground jobs (CTRL+C)
void kill_foreground_jobs();
//suspend all foreground jobs(CTRL+Z)
void suspend_foreground_jobs();
//list all background running jobs, their name, PID, job number with status
//also print exit status code of background jobs that just ended
void jobs();
//make specified job number in your list go to foreground
//resume exec if it is not running/stopped
//eg - fg 2
void fg(int job_num);
//cause specified program to run in background
//eg - bg 2
void bg(int job_num);

int* get_job_ids();
int* get_proc_ids();
int get_job_number();
void kill_all_jobs();


#endif