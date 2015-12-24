#include "job_control.h"

int init_job_control_funcs()
{
	//initialize the list to hold job control functions
	list_of_jobs = init_job_list();

	//initialize job ids and process ids - need to be one 
	//to allow for easy atoi use
	next_pid_number = 1;
	next_jid_number = 1;

	//return to silence fucking warning 
	return next_jid_number + next_pid_number;
}

//adds job to job list
//returns ID of job
int add_job(char *job_name, int job_id)
{
	//remove newline if name has one
	char *ptr = job_name;
	while(*ptr != '\0')
	{
		if(*ptr == '\n')
			*ptr = '\0';
		
		ptr++;
	}

	//STDOUT("ADDING JOB WITH NAME OF ");
	//STDOUT(job_name);
	//STDOUT(".\n");
	//for now, just use the command typed in as the job name, and auto assign JID and PID
	//job number v job id? - probably the same thing
	//int add_job_to_list(char* name, char* status, int process_id, int job_number, int job_id, int is_foreground, struct job_list *linked_list);
	add_job_to_list(job_name, "STATUS", next_pid_number, -1 /*<- this is job number */, job_id, 0, list_of_jobs);

	//STDOUT("added job to list in add_job\n");
	//job_id = next_jid_number;

	//increment PID and JID
	next_pid_number++;
	next_jid_number++;

	return EXIT_SUCCESS;
	//return job_id;
}

//removes all jobs
void kill_all_jobs()
{
	retire_all_processes(list_of_jobs);
}

//removes job from job list
void kill_job(char *job_name, int job_id)
{
	if(list_of_jobs->size <= 0)
		return;

	int * jobs = return_all_job_ids(list_of_jobs);
	int num_jobs = get_num_jobs(list_of_jobs);

	int i = 0;
	while(i < num_jobs)
	{
		if(*jobs == job_id)
		{
			//KILLING AND PILLAGING!!!
			kill(job_id, SIGKILL);

			retire_process(job_id, list_of_jobs);
			return;
		}
		i++;
		jobs++;
	}
}

//kill all foreground jobs (CTRL+C)
void kill_foreground_jobs()
{
	STDOUT("hit CTR+C\n");
}

//suspend all foreground jobs(CTRL+Z)
void suspend_foreground_jobs()
{
	STDOUT("hit CTRL+Z\n");
}

/*
All jobs or just backgrnd??
*/
//list all (background, running jobs,suspended, and zombies), their name, PID, job number with status
//also print exit status code of background jobs that just ended <- ?!?!?!!?!?!?!?!?!?
//list as "[JOB_ID] (PROCESS_ID) \t\t command_input<-(name)"
void jobs()
{
	//check pid and jid of all runnin processes to see if theyve died
        int* job_ids = get_job_ids();
        //int* process_ids =  get_proc_ids();
        int num_jobs = get_job_number();

        int i = 0;
    
        //if there are no jobs, clear the list
        if(num_jobs == 0)
        {
            kill_all_jobs();
        }
        else
        {
            while(i < num_jobs)
            {
                int child_status = -1;

                /*int wait_pid_ret = */waitpid(job_ids[i], &child_status, WNOHANG); 
                if(child_status ==0)
                {  
                    kill_job(NULL, job_ids[i]);
                }
                
                i++;
            }
         }
	struct job_node *ptr = list_of_jobs->head;

	//while we have jobs post their info to STDOUT
	while(ptr != NULL)
	{
		char conversion_buff[MAX_INT_CONVERSION_LENGTH];
		
		//print the job id 
		STDOUT("[");
		sprintf(conversion_buff, "%d", ptr->process_id);
		STDOUT(conversion_buff);
		STDOUT("] ");

		//print process id
		strcpy(conversion_buff, "");	//clear buffer
		STDOUT("(");
		sprintf(conversion_buff, "%d", ptr->job_id);
		STDOUT(conversion_buff);
		STDOUT(")\t\t");

		//print name of process
		STDOUT(ptr->name);
		STDOUT("\n");

		ptr = ptr->next_link;
	}
}

//make specified job number in your list go to foreground
//resume exec if it is not running/stopped
//eg - fg 2
void fg(int job_num)
{
	// STDOUT("in fg with job\n");

	if(list_of_jobs->size <= 0)
	{
		STDERR("ERR: No active jobs\n");
		return;
	}

	int * jobs = return_all_job_ids(list_of_jobs);
	int num_jobs = get_num_jobs(list_of_jobs);

	int i = 0;
	while(i < num_jobs)
	{
		if(*jobs == job_num)
		{
			int child_status =0;
			//int wait_return = 0;
			
			// Wait until child process is finished
			waitpid(*jobs, &child_status, 0);

			//child is free, kill
			kill_job(NULL, *jobs);

 			return;
		}

		i++;
		jobs++;
	}

	STDERR("ERR: Could not find job with id ");
	char conversion_buff[MAX_INT_CONVERSION_LENGTH];
	sprintf(conversion_buff, "%d", job_num);
	STDERR(conversion_buff);
	STDERR("\n");
}

//cause specified program to run in background
//eg - bg 2
void bg(int job_num)
{
	// STDOUT("in bg with job num ");

	char conversion_buff[MAX_INT_CONVERSION_LENGTH];
	sprintf(conversion_buff, "%d", job_num);
	STDOUT(conversion_buff);
	STDOUT("\n");
}

int get_job_number()
{
	return get_num_jobs(list_of_jobs);
}

int* get_job_ids()
{
	return return_all_job_ids(list_of_jobs);
}

int* get_proc_ids(struct job_list* list)
{
	return return_all_proc_ids(list_of_jobs);
}