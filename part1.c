#include "part1.h"
#include "part2.h"

void init_directories()
{
	getcwd(curr_directory, MAX_PROMPT);
	getcwd(prev_directory, MAX_PROMPT);
}

void trim_and_move_redundant_spaces(char *old_text)
{
	int i = 0;
	int spaces = 0;
	while (old_text[i] != '\0')
	{
		if (old_text[i] == ' ')
		{
			spaces++;

			if (old_text[i + 1] == '\0')
			{
				old_text[i] = '\0';
				return;
			}
		}
		else
			spaces = 0;

		if (spaces > 1)
		{
			// move everything over to the left one space
			// ls  -la \0
			//    i
			int j = i;
			while (old_text[j])
			{
				old_text[j] = old_text[j+1];
				j++;
			}

			spaces--;
		}
		else
		{
			i++;
		}

	}

	// print_stdout(old_text, "\n", 0);
}

void check_options(int argc, char **argv)
{
	int opt; // used to check flags and arguments

    /* Parse short options */
    while((opt = getopt(argc, argv, "d")) != -1) {
        switch(opt) {
            case 'd':
                /* Debug */
                debug = 1; // set debug to true
                break;
            case '?':
				//if there are only two arguments check to see if a shell script was given
				if(argc == 2)
				{	
					char second_param_copy[MAX_PROMPT];
					strcpy(second_param_copy, argv[1]);

					//if after this ptr is sh, then we have a shell script
					char*ptr = strtok(second_param_copy, ".");
					ptr = strtok(NULL, ".");
					if(strcmp(ptr, "sh") == 0)
					{
						char *exec_path = (char *) Malloc(MAX_PROMPT);
						char* path = "./";
						
						strcat(exec_path, path);
						strcat(exec_path, argv[1]);
						
						system(exec_path);
						
						free(exec_path);
						
						return;
					}
				}
            default:
                /* A bad option was provided. */
                exit(EXIT_FAILURE);
                break;
        }
    }
}

char ** initialize_envp(char **envp)
{
	char **envp320 = NULL;
    char *var = envp[0];

    // iterate through every environment varialble
    while (var)
    {
        // add this variable to list
        envp320 = (char**)Realloc(envp320, sizeof(char*) * (envp_size + 1) + 1);
		
		// realloc envp_320
		// just point each pointer to original envp,
		// since it is called in the main stack and will never disappear.
		envp320[envp_size] = var;

        var = envp[++envp_size];
     
    }  

    envp320[envp_size] = 0; // end with null

#ifdef DEBUG
    // print our shell's envp
    debug_envp(envp);
#endif

    return envp320;
}

bool execute_special_command(char *cursor , char *startPtr, char *last_line_poz, int index)
{
	char last_char = *cursor;
	switch(last_char)
	{
		case 3:		// Ctrl + C
					// \n and exit
					STDOUT("\n");
					kill_foreground_jobs();
					//exit(130);
					return true;
		case 26:	//CTRL + Z

					suspend_foreground_jobs();

					return true;
		case 127:	// Backspace
					// TODO
					has_backspaced = 1;
					if(cursor == startPtr && strcmp(old_text, "") == 0)
						return true;
					
					STDOUT("\n");

					//we have entered new test, remove from there.
					int removing_old_text = 0;
					if(cursor != startPtr)
					{
						// STDOUT("Removing new text\n");

						char *start_dummy = startPtr;
						char *end_dummy = cursor;
						end_dummy--;

						while(start_dummy < end_dummy)
							start_dummy++;
						*start_dummy = '\0';
					}
					//we have not entered new text, we should instead remove from old text
					else
					{
						removing_old_text = 1;
						// STDOUT("Removing old text\n");
						char *end_dummy = old_text;

						while(*end_dummy != '\0')
							end_dummy++;
						end_dummy--;
						*end_dummy = '\0';
					}
					
					STDOUT(promptHead);
			        STDOUT(pwd);
        			STDOUT(promptTail);

        			//TODO: Fix some problem happening here
        			STDOUT(old_text);
        			if(removing_old_text == 0)
        			{
						STDOUT(startPtr);
						strcat(old_text, startPtr);
						strcat(old_text, "\0");
						last_line_poz[index] = '\0';
					}

					return true;
		case 9:		// Tab
					// TODO: extra credit
					// STDOUT("Tab\n"); // temp
					return true;		

        case '\033':// escape char

        			// check if Up or Down char
        			Read(0, cursor, 1); // read a '['
        			Read(0, cursor, 1); // read A or B
        			last_char = *cursor;

        			switch(last_char)
        			{
        				case 'A':	// Up
			        				// TODO
        							page_up(); 
        							STDOUT("\n");  
        							return true;  


        				case 'B':	// Down
        							// TODO
        							page_down();
        							STDOUT("\n");
        							return true;

        				case 'C':	// Right
        							// TODO
        							// STDOUT("Right\n"); // temp
        							return true;

        				case 'D':   // Left
        							// TODO
        							// STDOUT("Left\n"); // temp
        							return true;
        			}
	}

	return false;
}

//note we need called recursively since program reads executable 
void execute_command(char *cmd, char *envp[], int called_recursively)
{
	int job_id = -1;

	if(just_paged == 1)
		return;
	
	// Since cmd will be tokenized after initialize_argv is called,
	// create a copy here without the newline char to output to
	// $HOME/.320sh_history
	char cmd_string[strlen(cmd) + 1];
	strcpy(cmd_string, cmd);
	cmd_string[strlen(cmd)] = '\0'; // remove \n

	// First, tokenize cmd into whitespace delimited array of strings.
	int argc = 0;							 	// number of arguments
	char **argv = initialize_argv(cmd, &argc);	// free this when finished

	// Replace all variables with their bindings
	char *variable_buffer[MAX_VAR];
	// keep track of how many indices of variable_buffer we use and thus need
	// to free.
	int indices_used = 0;
	int i = 0; // index later to be used in for loop

	// If the input is only spaces, exit.
	if (argc == 0)
		//this is hit when something is just executed without ./ and with ./
		goto done;
	
	// put this command in $HOME/.320sh_history
	if(is_reading_script == 0)
		save_to_history(cmd_string);
#ifdef DEBUG
	// print argv
	debug_argv(argc, argv);
#endif

// From here we need to handle:
// 1. Replacing variable names with bindings
// 2. Checking if a process should run on foreground vs background.
// 3. Checking for pipes
// 4. Checking if command is built-in.
// 5. Checking if command exists as binary file in relative path.
// 6. Checking if command exists as binary file in PATH.

	// replace all variable names found in argv with their bindings in envp320
	// If a binding does not exist, replace it with the empty string.
	replace_variables(argc, argv, variable_buffer, &indices_used);
	if (argc == 1 && strcmp(argv[0], "") == 0) // variable was undefined
	{
		goto done;
	}
	// Check if argv[0] is a built-in command
	// If yes, execute it with arguments in argv[1+] and return.
	// if (executeBuiltIn(argv)) ...
	// TODO

	char *exec_path = (char *) malloc(MAX_PROMPT);
	
//hacky hacky hacky
	if(strcmp(*argv, "set") == 0 || strcmp(*argv, "exit") == 0 || strcmp(*argv, "history") == 0 || strcmp(*argv, "clear-history") == 0 ||
	strcmp(*argv, "pwd") == 0 || strcmp(*argv, "jobs") == 0 || strcmp(*argv, "bg") == 0 || strcmp(*argv, "fg") == 0 || strcmp(*argv, "kill") == 0 ||
	strcmp(*argv, "cd") == 0)
		goto chain;

	//if only one argument, it can only be the shell script, try seeing it if its
	if(argc == 1)
		strcpy(exec_path, argv[0]);
	else if(argc == 2)
	{
		//char *dummy = argv[1];
		strcpy(exec_path, argv[1]);
	}
	
	//if we are executing exit command, just exit
	if(strcmp(exec_path, "exit") == 0)
		exit(123);

	//if we are not executing cd/ls
	if(strcmp(exec_path, "..") != 0)
	{
		FILE *fp = fopen(exec_path, "r");
    	if(fp == NULL)
    	{
    	  	free(exec_path);
    	   	goto chain;
    	}
    	else
    	{
    		is_reading_script = 1;
    		// STDOUT("READING SCRIPT\n");
    		char * line = (char *) malloc(MAX_DIRECTORY_LENGTH);
    		size_t len = MAX_DIRECTORY_LENGTH;
    		int read = 0;
    		while ((read = getline(&line, &len, fp)) != -1) 
    		{
    			//STDOUT("IN LOOP\n\n");
    			//printf("%s", line);
    		   //if we read a hashban
    		   if(line[0] == '#' && line[1] == '!' && reading_first_line == 1)
    		   {
    		   		in_scipt = 1;
    		   		reading_first_line = 0;

    		   		//add job
    		   		//if this was not called recursively, add it to the job list
					if(called_recursively == 0)
					{
						//STDOUT("Add job called at 334\n");
						//add command to the job list
						//job_id = add_job(cmd);

						//STDOUT("ASSIGNED3 JOB WITH ID ");
    	   				char conversion_buff[MAX_INT_CONVERSION_LENGTH];
						sprintf(conversion_buff, "%d", job_id);
						STDOUT(conversion_buff);
						STDOUT("\n");
				
					}
	
    		   }
    		   //if we are in the script and past the first line
    		   else if(in_scipt == 1 && reading_first_line == 0)
    		   {
					if(line[0] == '#')
    	       		{
    	       			reading_first_line = 0;
    	       			// STDOUT("read pound\n");
    	       			continue;
    	       		}
    	       		else //we must be reading a command
    	       		{
    	       			// STDOUT("EXECUTING CMD\n");
    	       			execute_command(line , envp, 1);
    	       			// STDOUT("RETURNED FROM EXECUTE CMD\n");
    	       	 	}
    	       	}
    	       else if(in_scipt == 0)
    	       {
    	       	 //in a gcc executable, first line is ELF
    	       	 //see http://man7.org/linux/man-pages/man5/elf.5.html
    	       	if(strcmp(line, "ELF"))
    	       	{
    	       		//STDOUT("READ ELF\n");
    	       		if (execute_binary(argv, envp, called_recursively, argc, job_id))
					{
					/*	char **ptr = argv;
						char * input = (char *)malloc(MAX_DIRECTORY_LENGTH);
						int i = argc;
						while(i > 0)
						{
							strcpy(input, *ptr);
							ptr++;
							i--;

							strcpy(input, " ");
						}
						STDOUT("Input was: ");
						STDOUT(input);
						STDOUT("\n");

						if(called_recursively == 0)
							add_job(cmd);*/

						// command was executed.
						//STDOUT("Command was binary\n");
						goto done;
					}


					// Else, alert the user that the command is not found.
					STDERR("found ELF");
					STDERR(argv[0]);
					STDERR(": command not found\n");

					//remove it from job list
					//STDOUT("killing bad job\n");
					//STDOUT("KILLING ID ");
                    //char conversion_buff[MAX_INT_CONVERSION_LENGTH];
                    //sprintf(conversion_buff, "%d", job_id);
                    //STDOUT(conversion_buff);
                    //STDOUT("\n");
                    
					kill_job(NULL, job_id);

					goto done;
    	       	}
    	       	else
    	       	{
    	       	 	execute_command(line , envp, 1);
    	       	 	//STDOUT("Returned from recursive execute\n");
    	       	 	//STDOUT("command is ");
    	       	 	//STDOUT(argv[0]);
    	       	 	//STDOUT("\n");
    	       	 	free(exec_path);
    	       	 	goto done;
    	       	 }
    	       }
    	   }
    	   //if we assigned a job id to script retireit
    	   //STDOUT("KILLING JOB (SCRIPT)\n");

    	   if(job_id != -1)
    	   {
    	   		//STDOUT("KILLING JOB WITH ID ");
    	   		//char conversion_buff[MAX_INT_CONVERSION_LENGTH];
				//sprintf(conversion_buff, "%d", job_id);
				//STDOUT(conversion_buff);
				//STDOUT("\n");

    	   		kill_job(NULL, job_id);
    	   }
    	   is_reading_script = 0;
    	}

    	//STDOUT("Execve'ing\n");
    	execve(exec_path, argv, envp320);
    
    	free(exec_path);
		goto done;
	}
chain:
// STDOUT("in chain\n");
// STDOUT("arg is ");
// STDOUT(*argv);
// STDOUT("\n");
	//if the command is set, set a variable
	if (strcmp(*argv, "set") == 0)
	{
		// STDOUT("In set\n");
		// case 'set name=binding'
		// 
		// case 'set name= binding'
		// 
		// case 'set name =binding'
		// 
		// case 'set name = binding'
		// 
		int i = 1;
		int size = 0;
		for (; i < argc; i++)
		{
			size += (1 + strlen(argv[i]));
		}

		char set_argument[size];

		i = 1;
		int j = 0;
		for (; i < argc; i++)
		{
			sprintf(set_argument + j, "%s ", argv[i]);
			j += (1 + strlen(argv[i]));
		}

		// replace last space with \0
		set_argument[strlen(set_argument) - 1] = 0;

		// print_stdout(set_argument, "$\n", 0);
		// Now set_argument contains name equals binding as one string.
		char* addr_of_equals = strstr(set_argument, "=");
		if (addr_of_equals == 0) // no equals sign
			goto done;

		int index_of_equals = addr_of_equals - set_argument;

		set_argument[index_of_equals] = '\0'; // set the = sign to null

		// contains space? remove it
		char *addr_of_space = strstr(set_argument, " ");
		if (addr_of_space != 0)
			*addr_of_space = '\0';

		// This is our variable name
		char *name = set_argument;

		char *binding = set_argument + index_of_equals + 1;

		addr_of_space = strstr(binding, " ");
		if (addr_of_space == addr_of_equals + 1)
		{
			binding++;
		}

		//print_stdout(name, "\n", binding, "\n", 0);

		// name has variable name
		// binding has variable binding
		// Check to see if this name exists in envp
		// 	If yes, update it.
		// 	If no, append new entry
		i = 0;
		char *iter = envp320[i];
		for (; i <= envp_size; i++)
		{
			index_of_equals = strstr(iter, "=") - iter;
			char compare[index_of_equals + 1];
			int j = 0;
			for (; j < index_of_equals; j++)
			{
				compare[j] = iter[j];
			}
			compare[j] = '\0';

			if (strcmp(name, compare) == 0)
			{
				// name exists,
				// update mapping
				new_var_mapping = (char**)Realloc(new_var_mapping, sizeof(char*) * (new_map_size + 1));
				new_var_mapping[new_map_size] = (char *)Malloc(sizeof(char) * (strlen(name) + strlen(binding) + 2));
				sprintf(new_var_mapping[new_map_size], "%s=%s", name, binding);

				// print_stdout(new_var_mapping[new_map_size], "\n", 0);
				if (i != 0)
					envp320[i - 1] = new_var_mapping[new_map_size];
				else
					envp320[0] = new_var_mapping[new_map_size];

				new_map_size++;

				// debug_envp(envp320);

				goto done;
			}

			iter = envp320[i];
		}

		// name doesn't exist,
		// create mapping
		new_var_mapping = (char**)Realloc(new_var_mapping, sizeof(char*) * (new_map_size + 1));
		new_var_mapping[new_map_size] = (char *)Malloc(sizeof(char) * (strlen(name) + strlen(binding) + 2));
		sprintf(new_var_mapping[new_map_size], "%s=%s", name, binding);

		envp320 = (char**)Realloc(envp320, sizeof(char*) * (envp_size + 1) + 1);
		envp_size++;

		// print_stdout(new_var_mapping[new_map_size], "\n", 0);
		envp320[envp_size - 1] = new_var_mapping[new_map_size];
		envp320[envp_size] = 0;

		new_map_size++;

		// debug_envp(envp320);

		goto done;
	}

	//if the command is exit, exit the shell
	if(strcmp(*argv, "exit") == 0)
	{
		// STDOUT("In exit\n");
		exit(130);
	}
	
	//if the command is history, execute command history
	else if(strcmp(*argv, "history") == 0)
	{
		// STDOUT("In history\n");
		execute_history();
		goto done;
	}
	
	//if the command is "clear-history", clear the command history
	else if(strcmp(*argv, "clear-history") == 0)
	{
		// STDOUT("In clear-history\n");
		clear_history();
		goto done;
	}

	//if the command is pwd, list path to working directory
	else if(strcmp(*argv, "pwd") == 0)
	{
		//create buffer and allocate space for it
		size_t buff_size = MAX_PROMPT;
		char *buffer = (char*)malloc(buff_size);

		//print current directory
		STDOUT(getcwd(buffer, buff_size));
		STDOUT("\n");

		//free buffer
		free(buffer);

		goto done;
	}

	else if(strcmp(*argv, "jobs") == 0)
	{
		//STDOUT("Command was jobs\n");
		//print all jobs to stdout
		jobs();
		goto done;
	}

	else if(strcmp(*argv, "bg") == 0)
	{
		//STDOUT("Command was bg\n");
		
		//if no job provided, print error and return
		if(argv[1] == NULL)
		{
			STDOUT("Need job number\n");
			goto done;
		}
		else
		{
			char *job = argv[1];
			if(job[0] != '%')
			{
				STDOUT("Job improperly formatted\n");
				goto done;
			}
			else
			{
				//go past %
				job++;

				//convert job num to int
				int job_num = atoi(job);
				if(job_num == 0)
				{
					STDOUT("Job improperly formatted\n");
					goto done;
				}

				bg(job_num);
				goto done;
			}
		}
	}

	else if(strcmp(*argv, "fg") == 0)
	{
		//STDOUT("Command was fg\n");

		if(argv[1] == NULL)
		{
			STDOUT("Need job number\n");
			goto done;
		}
		else
		{
			char *job = argv[1];
			if(job[0] != '%')
			{
				STDOUT("Job improperly formatted\n");
				goto done;
			}
			else
			{
				//go past %
				job++;

				//convert job num to int
				int job_num = atoi(job);
				if(job_num == 0)
				{
					STDOUT("Job improperly formatted\n");
					goto done;
				}

				fg(job_num);
				goto done;
			}
		}
	}
	//kill command
	else if(strcmp(*argv, "kill") == 0 || strcmp(*argv, "KILL") == 0)
	{
		//STDOUT("detected kill call\n");
		kill_job(NULL, atoi(argv[1]));

		goto done;
	}

	//first argv is command
	//if the first command is cd then we must change directory
	//no need to fork
	else if(strcmp(*argv , "cd") == 0)
	{
		//STDOUT("Command was cd\n");
		//if we are implementing '-' command - change to last
		//working directory
		if(argv[1] == NULL)
		{
		//	STDOUT("going to done\n");
			goto done;
		}
		else if(strcmp(argv[1],"-") == 0)
		{
			//STDOUT(prev_directory);
			//STDOUT("\n");
			//STDOUT(curr_directory);
			//STDOUT("\n");
			
			//use temp to change directory
			char change_dir[100];
			strcpy(change_dir, prev_directory);

			getcwd(prev_directory, MAX_DIRECTORY_LENGTH);
			
			chdir(change_dir);
			
			//store current directory for use in cd -
			//strcpy(prev_directory, curr_directory);
			
			
			getcwd(curr_directory, MAX_DIRECTORY_LENGTH);

			
			goto done;
		}
		//go back one directory
		else if(strcmp(argv[1], "..") == 0)
		{
			//STDOUT(prev_directory);
			//STDOUT("\n");
			//STDOUT(curr_directory);
			//STDOUT("\n");
			
			getcwd(prev_directory, MAX_DIRECTORY_LENGTH);
			//strcpy(prev_directory, curr_directory);
			getcwd(curr_directory, MAX_DIRECTORY_LENGTH);

			char *ptr = curr_directory;
			
			//STDOUT("ptr is ");
			//STDOUT(ptr);
			//STDOUT("\n");
			while(*ptr != '\0')
			{
				ptr++;
			}
			while(*ptr != '/')
			{
				ptr--;
			}
			*ptr = '\0';
			
			//copy it to previous directory
			
			//strcpy(prev_directory, curr_directory);
			chdir(curr_directory);

			goto done;
		}	

		char temp_dir[MAX_DIRECTORY_LENGTH];
		strcpy(temp_dir, prev_directory);
		getcwd(prev_directory, MAX_DIRECTORY_LENGTH);
		
		//second parameter must be directroy to change to
		//if -1 ret val, error occured
		if(chdir(argv[1]) == -1)
		{
			STDERR(argv[0]);
			STDERR(": ");
			STDERR(argv[1]);
			STDERR(": No such file or directory\n");

			//reset previous directory
			strcpy(prev_directory, temp_dir);
		}
		
		else
		{

			//save curent directory as orevious
			//strcpy(prev_directory, curr_directory);
			
			//chdir(argv[1]);
			
			getcwd(curr_directory, MAX_DIRECTORY_LENGTH);

			//STDOUT("CHANGED DIR\n");
			//STDOUT(prev_directory);
			//STDOUT("\n");
			//STDOUT(curr_directory);
			//STDOUT("\n");
		
		}
		goto done;
	}
	//else add the jobsince its not built int
	else
	{
		//if this was not called recursively, add it to the job list
		if(called_recursively == 0)
		{
			//STDOUT("Add job called at 711\n");
						
			//add command to the job list
			//job_id = add_job(cmd);
		}
	}
	// Else, check if argv[0] is a binary file located in the PATH environment 
	// variable.
	// If yes, execute it with arguments and return.
	if (execute_binary(argv, envp, called_recursively, argc, job_id))
	{
		//STDOUT("EXECUTING BINARY");
		// command was executed.
		//STDOUT("Command was binary\n");
		
		char **ptr = argv;
		char * input = (char *)malloc(MAX_DIRECTORY_LENGTH);
		int i = argc;
		while(i > 0)
		{
			strcpy(input, *ptr);
			ptr++;
			i--;

			strcpy(input, " ");
		}
		//STDOUT("Input was: ");
		//STDOUT(input);
		//STDOUT("\n");
						
	
		// command was executed.
		//STDOUT("Command was binary\n");

		//add command to the job list - check if command is ls - since its built in, it will get added when it should not
		if(called_recursively == 0)
		{
			char name[MAX_DIRECTORY_LENGTH];
			strcat(name, old_text);
			strcat(name, cmd);
				
			if(strcmp(old_text, "ls") != 0 && strcmp(cmd, "ls") != 0)
			{
				//STDOUT("Add job called at 754\n");
						
				//job_id = add_job(name);
			}
			
			//STDOUT("old text: ");
			//STDOUT(old_text);
			//add_job(cmd);
		}

		goto done;
	}
	//STDOUT("EXECUTED BINARY\n");
		//if this was not called recursively, add it to the job list

	
	// Else, alert the user that the command is not found.
//	STDOUT("regular execute binary\n");
	STDERR(argv[0]);
	STDERR(": command not found\n");

	//STDOUT("killing bad job\n");
	//STDOUT("KILLING ID ");
      //              char conversion_buff[MAX_INT_CONVERSION_LENGTH];
        //            sprintf(conversion_buff, "%d", job_id);
          //          STDOUT(conversion_buff);
            //        STDOUT("\n");
                    
	kill_job(NULL, job_id);


done:
	//STDOUT("In done\n");
	// free variable_buffer
	for (; i < indices_used; i++)
	{
		free(variable_buffer[i]);
	}

	free(argv);
}

char **initialize_argv(char *cmd, int *argc)
{
	char *arguments = strtok(cmd, " \n\t");

	char **argv = NULL;
	*argc = 0;

	// tokenize the arguments
	//if there is a quation mark, we need to know 
	int has_quotation_mark = 0;
	int read_quoted_value = 0;
	int num_groups_skipped = 0;
	int has_just_read_quotation = 0;
	char *command = (char *)calloc(MAX_PROMPT, sizeof(char));
	//put in null term to show empty array
	//*command = '\0';
	while (arguments) {
		char *ptr = arguments;
		
		while(*ptr != '\0')
		{
			if(*ptr == '"')
			{
				has_just_read_quotation = 1;
				if(has_quotation_mark == 0)
				{
					has_quotation_mark = 1;
					//we found a paren so copy every after paren to buffer 
					//for later execution
					ptr++;
					strcat(command, ptr);
					break;
				}
				else
				{
					has_just_read_quotation = 1;
					//we have a closing parenthesis, so copy everything before 
					//it to buffer
					char*copy_pointer = arguments;
					while(*copy_pointer != '"')
						copy_pointer++;
					*copy_pointer = '\0';

					strcat(command, " ");
					strcat(command, arguments);
					
					//remove off by one error in count
					(*argc)--;

					has_quotation_mark = 0;
					read_quoted_value = 1;
					break;
				}
			}
			ptr++;
		}
		if(has_quotation_mark == 1 && has_just_read_quotation != 1)
		{
			
			strcat(command, arguments);
			num_groups_skipped++;
		}
		has_just_read_quotation = 0;

		// realloc arg array
		argv = (char**)Realloc(argv, sizeof(char*) * (*argc + 1));
		//if there is not a trailing opening parenthesis, add in argument
		if(read_quoted_value == 0)//has_quotation_mark == 0)
		{
			if(strcmp(arguments, "&") != 0)
			{
				is_background_process = 0;
				argv[(*argc)++] = arguments;
			}
			else
				is_background_process = 1;//flag process as background process
		}
		//if we are entering a special quoted value
		else if(read_quoted_value == 1)
		{
			if(strcmp(arguments, "&") != 0)
			{
				is_background_process = 0;
				argv[(*argc)++] = command;
			}
			else
				is_background_process = 1;//flag process as background process

			num_groups_skipped--;
			while(num_groups_skipped > 0)
			{
				//(*argc)++;
				num_groups_skipped--;
			}

			//no longer needed - remove
			strcpy(command, "");
		}

		// TODO: Arguments between quotes should be seen as one argument.
		arguments = strtok(NULL,  " \n\t"); // retokenize
	}
		
	// Make sure the input isn't just whitespace
	if (*argc > 0)
	{
		// Add a NULL at the end of this array for exec functions
		// realloc arg array
		argv = (char**)Realloc(argv, sizeof(char*) * (*argc + 1));

		argv[(*argc)] = 0;
	}

	return argv;
}

void replace_variables(int argc, char **argv, char *variable_buffer[MAX_VAR], int *indices_used)
{
	// Ex:
	// 320sh> set DUDE=world
	// 320sh> echo hello$DUDE
	// helloworld

	// iterate through argv,
	int i = 0;
	for (; i < argc; i++)
	{
		char *argument = argv[i];

		// check for all '$'s
		char *$_loc = strstr(argument, "$");

		if ($_loc)
		{
			// we have a variable name at location *$_loc until a ' ' or '\n' is seen
			int start_index = $_loc - &argument[0];
			int end_index = start_index + 1;

			while (argument[end_index] && argument[end_index] != ' ' && argument[end_index] != '\n')
			{
				end_index++;
			}

			char name[end_index - start_index + 1]; // variable name

			// copy over name not including '$'
			int j = 0;
			int k = start_index + 1;
			while (k != end_index + 1)
			{
				name[j] = argument[k];
				j++;
				k++;
			}
			name[j] = '\0';

			argument[start_index] = '\0'; // we will use this to create our new string.

			// now var contains the variable without a '$'
			// Search envp320 for name's binding and map it to the variable.
			char *binding = NULL; // default binding
			binding = get_binding(name);

			// Set argument into variable_buffer
			variable_buffer[*indices_used] = (char*)Malloc(strlen(binding) + strlen(argument) + 1);

			sprintf(variable_buffer[*indices_used], "%s%s", argument, binding);

			argv[i] = variable_buffer[*indices_used];
			indices_used++;

			free(binding);
			
		}
	}
}

char *get_binding(const char *name)
{
	// $? binds to previous return code
	if (strcmp(name, "?") == 0)
	{
		char *binding = (char*)Malloc(4); // up to 3 digits

		sprintf(binding, "%d", errno);

		return binding;
	}

	// search envp320 for a string of that starts with the string in name
	// followed by an equals sign.
	char name_equals[strlen(name) + 2]; // name + equal sign (NAME=)
	sprintf(name_equals, "%s=", name);

	char *iter = envp320[0];
	int i = 0;
	while(iter)
	{
		if (strstr(iter, name_equals) == iter) // found name=
		{
			char *binding = (char*)Malloc((sizeof(char)) * strlen(&iter[strlen(name) + 1]) + 1);
			strcpy(binding, &iter[strlen(name) + 1]);
			//print_stdout(binding, "$\n", 0);
			
			return binding;
		}

		// iterate
		i++;
		iter = envp320[i];
	}

	char *binding = (char*)Malloc(1);
	strcpy(binding, "");
	return binding;
}

void save_to_history(char *cmd_string)
{
	// create concatenated string representing $HOME/.320sh_history
    char history_path[sizeof(get_binding("HOME")) + sizeof(HISTORY) + 2];
    sprintf(history_path, "%s/%s", get_binding("HOME"), HISTORY);

    // open $HOME/.320sh_history
    ofp = fopen(history_path, "a");
	
	if (!piping || pipes_left == 0)
    	fprintf(ofp, "%s", whole_cmd);

    // close $HOME/.320sh_history
    fclose(ofp);
}

bool execute_binary(char *argv[], char *envp[], int called_recursively, int argc, int job_id)
{
	//STDOUT("ABOUT TO EXECUTE BINARY\n");
	// To make things easier
	char *command = argv[0];
	//STDOUT("BINARY COMMAND IS ");
	//STDOUT(argv[0]);
	//STDOUT("\n");

	// First, check if the command has '/' in it, which signifies that it is a
	// path to a file. If the command has a '/', then check if that path 
	// exists. If yes, create a child process and execute the command there.
	if (strstr(command, "/") != 0 || strstr(command, "./")) // if '/' is in command
	{
		if (file_exists(command))
		{
			char **ptr = argv;
			char * input = (char *)malloc(MAX_DIRECTORY_LENGTH);
			int i = argc;
			while(i > 0)
			{
				strcpy(input, *ptr);
				ptr++;
				i--;

				strcpy(input, " ");
			}
			
		//	STDOUT("Input was: ");
		//	STDOUT(input);
		//	STDOUT("\n");

			if(called_recursively == 0)
			{
				char name[MAX_DIRECTORY_LENGTH];
				strcat(name, old_text);
				strcat(name, cmd);

				if(strcmp(old_text, "ls") != 0 && strcmp(cmd, "ls") != 0)
				{
					//STDOUT("Added job at 1063\n");
					//job_id = add_job(name);
				}
				
//				STDOUT("old text: ");
//				STDOUT(old_text);
//				add_job(cmd);
			}
			
			// fork and execute
			//STDOUT("forking\n");
			execute_in_child_process(command, argv, envp, job_id, called_recursively);
			
			return true;
		}

		// Return false if the command is given with a path and is not found.
		return false;
	}

	// If we reached here, the command does not have a '/' in it.
	// We must look through each path in $PATH to see if this file exists.
	
	// $PATH
	// don't forget to free the address in path.
	//STDOUT("past if\n");
	
	char *path = (char*)Malloc(sizeof(char) * strlen(get_binding("PATH")) + 1);
	
	strcpy(path, get_binding("PATH"));
	path = strtok(path, ":");

	// this is so we can free this reference this address later since strtok
	// changes the value of path.
	char *free_me = path; 
	//STDOUT("made path\n");
	
	// Iterate through each path and append the command name to it.
	// Then, check if this file exists. If yes, create a child process and
	// execute the command there.
	while (path != NULL) {

		// append path + / + command into a new string
		char path_to_command[strlen(path) + strlen(command) + sizeof(char)];
		sprintf(path_to_command, "%s/%s", path, command);

		//STDOUT("ABOUT TO CHECK IF FILE EXISTS\n");
		if (file_exists(path_to_command))
		{
		//	STDOUT("file exists\n");
	
			char **ptr = argv;
			char * input = (char *)malloc(MAX_DIRECTORY_LENGTH);
			int i = argc;

			while(i > 0)
			{
				strcpy(input, *ptr);
				ptr++;
				i--;

				strcpy(input, " ");
			}
			

			//STDOUT("Input was: ");
			//STDOUT(input);
			//STDOUT("\n");
	
			if(called_recursively == 0)
			{
				//STDOUT("called recursively\n");
				char name[MAX_DIRECTORY_LENGTH];
				strcat(name, old_text);
				strcat(name, cmd);
				//STDOUT("make name\n");
				if(strcmp(old_text, "ls") != 0 && strcmp(cmd, "ls") != 0)
				{
					//STDOUT("Add job called at 1132\n");
						
					//job_id = add_job(name);
				}
				//STDOUT("old text: ");
				//STDOUT(old_text);
				//add_job(cmd);
			}
			//STDOUT("file does not exist");
			//exit(123);
			// fork and execute
			//STDOUT("fork at 1113\n");
			execute_in_child_process(path_to_command, argv, envp, job_id, called_recursively);
			free(free_me);
			return true;
		}
		
		path = strtok(NULL, ":"); // retokenize
	}

	//STDOUT("about to free\n");
	
	free(free_me);
	return false;
}

bool file_exists(char *file)
{
	// use stat to check if a file exists
	struct stat st;
    int result = stat(file, &st);

    // check to see if the file exists
    if (result == 0) {
    	// Exists!
    	// Check if this file is actually a directory.
    	if (S_ISREG(st.st_mode) == 0)
    	{
    		// this is a directory
    		print_stderr("320sh: ", file, ": Is a directory\n", 0);
    	}

    	// Return true whether or not the "file" we found is a file or dir
      	return true;
    }

    return false;
}

void execute_in_child_process(char *command, char *argv[], char *envp[], int job_id, int called_from_script)
{
	if(is_background_process == 1)
	{
		execute_in_child_process_nonblocking(command, argv, envp, job_id, called_from_script);
		if (pipes_left != 0)
			return;
	}

	// Execute this in a child process
	// STDOUT("forking now blocking\n");
	pid_t child_pid;
	child_pid = Fork();
	int child_status;
	
	if (child_pid == 0)
	{
		// STDOUT("CHILD EXEC BLOCK\n");

		// Change file descriptors for piping if we need to
		change_piping_descriptors();
		
		// // Change file descriptors for redirection if we need to
		change_redirect_descriptors();

		// Everything in this scope is executed by only the child.
		execve(command, argv, envp); // execv terminates program after it is called.

		exit(EXIT_FAILURE); // Command failed to execute because argv or envp was incorrect
	}
	else
	{
		//STDOUT("PARENT EXEC BLOCK\n");
		// Everything in this scope is executed by only the parent.
		pid_t tpid;
			
		// if shell was executed with -d
		// before child process finishes
		// "RUNNING: [cmd]"
		if (debug)
 		{
			if (piping)
			{
				if (is_first_command)
					print_stdout(YELLOW, "RUNNING:", NOCOLOR, " ", command, "\n", 0);
			}
			else	
				print_stdout(YELLOW, "RUNNING:", NOCOLOR, " ", command, "\n", 0);
		}
	
		do {
			// Wait until child process is finished
   			tpid = Wait(&child_status);
 		} while(tpid != child_pid && is_background_process != 1);

 		reset_redirections();

 		// if shell was executed with -d
 		if (debug)
 		{
 			// after child process finishes
 			// "ENDED: cmd (ret=%d)"
 			char status[(child_status / 10) + 1];
 			sprintf(status, "%d", child_status);

			if (piping)
			{
				if (pipes_left == 0)
				{
					print_stdout(YELLOW, "ENDED:" NOCOLOR, "   ", command, " (ret=", 0);
					STDOUT(status);
	 				print_stdout(")", "\n", 0);
				}
			}
			else
			{
	 			print_stdout(YELLOW, "ENDED:" NOCOLOR, "   ", command, " (ret=", 0);
	 			STDOUT(status);
	 			print_stdout(")", "\n", 0);
	 		}

 		}

 		just_paged = 0;

 		//remove job_id from job list
 		//STDOUT("KILLING ID ");
          //          char conversion_buff[MAX_INT_CONVERSION_LENGTH];
            //        sprintf(conversion_buff, "%d", job_id);
              //      STDOUT(conversion_buff);
                //    STDOUT("\n");
                    
 		kill_job(NULL, job_id);

// 		STDOUT("RESET OLD TEST LINE 1209 pt1.c\n");
// 		STDOUT("OLD TEXT WAS ");
// 		STDOUT(old_text);
// 		STDOUT("\n");
 		//STDOUT("copying into old test 1301 part1.c\n");
    	strcpy(old_text, "");

    	if(called_from_script == 0 && pipes_left == 0)
    	    main_loop();
	}
}

void execute_in_child_process_nonblocking(char *command, char *argv[], char *envp[], int job_id, int called_from_script)
{
	//STDOUT("should be nonblocking\n");
	if (debug)
		print_stdout(YELLOW, "RUNNING:", NOCOLOR, " ", command, "\n", 0);

	//0 is false, 1 is true
	//int child_has_update = 0;

	//pids are set properly 
	pid_t child_pid;
	child_pid = Fork();

	if (child_pid == 0)
	{
		/*
		DO NOT UNCOMMENT THIS - IT IS ALMOST CERTAINLY WRONG
		*/
		//if(setpgid(0, next_pid_number - 1) == -1)
		//{
		//	STDOUT("Errno is: ");
		//	char status[100];
 		//	sprintf(status, "%d", errno);
 		//	STDOUT(status);
		//
		//			STDOUT("Could not set ids\n");
		//}
		//else
		//{
		//	;//	STDOUT("set fine");
		//}
		// Change file descriptors for piping if we need to
		change_piping_descriptors();

		// Change file descriptors if we need to
		change_redirect_descriptors();

		//child_has_update = 1;
		// Everything in this scope is executed by only the child.
		execve(command, argv, envp);//, envp); // execv terminates program after it is called.
	
		//remove job if argv or envp is incorrect
 		kill_job(NULL, job_id);

		exit(EXIT_FAILURE); // Command failed to execute because argv or envp was incorrect
	}
	else
	{
		//the child pid is the child process
		add_job(command, child_pid);

		//while(child_has_update == 0){ }
		reset_redirections();
		// Everything in this scope is executed by only the parent.
		//STDOUT("Is parent\n");
		
		//remove job if exit
		//assign the pid and jid properly to child
		int child_status;
		if(waitpid(0, &child_status, WNOHANG))
		{
			//STDOUT("KILLING ID ");
              //      char conversion_buff[MAX_INT_CONVERSION_LENGTH];
                //    sprintf(conversion_buff, "%d", job_id);
                  //  STDOUT(conversion_buff);
                   // STDOUT("\n");
                    
	 		kill_job(NULL, job_id);
		}
		

		//while(1)
		//{
		//	STDOUT("parent\n");
		//}
		// if shell was executed with -d
		// before child process finishes
		// "RUNNING: [cmd]"
		if (debug)
		{
			if (piping)
			{
				if (is_first_command)
					print_stdout(YELLOW, "RUNNING:", NOCOLOR, " ", command, "\n", 0);
			}
			else	
				print_stdout(YELLOW, "RUNNING:", NOCOLOR, " ", command, "\n", 0);
		
			// after child process finishes
 			// "ENDED: cmd (ret=%d)"
 			//char status[(child_status / 10) + 1];
 			//sprintf(status, "%d", child_status);

			if (piping)
			{
				if (pipes_left == 0)
				{
					print_stdout(YELLOW, "ENDED:" NOCOLOR, "   ", command, " (ret=", 0);
	 				print_stdout(")", "\n", 0);
				}
			}
			else
			{
	 			print_stdout(YELLOW, "ENDED:" NOCOLOR, "   ", command, " (ret=", 0);
	 			print_stdout(")", "\n", 0);
	 		}
 		}

 		//remove old text
 		just_paged = 0;
 		//STDOUT("reset old text line 1185 pt1.c\n");
 		strcpy(old_text, "");

 		if(called_from_script == 0 && pipes_left == 0)
    	    main_loop();
	}
}