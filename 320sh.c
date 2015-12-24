#include "320sh.h"

#ifndef __PART1FUNCTIONS_H
	#include "part1.h"
#endif

int main(int argc, char ** argv, char **envp) 
{
    // STDOUT("called main\n");
    init_directories();
    initialize_variables(argc, argv, envp);
    main_loop();
    return 0;
}

void initialize_variables(int argc, char ** argv, char **envp)
{
    //value of 0 means we are not in script, 1 means we are in script
    in_scipt = 0;
    //we initialy are reading the first line, should be initialized to 1
    reading_first_line = 1;

    //copy over variables passed in
    shell_argc = argc;
    shell_argv = argv;
    shell_envp = envp;

        //initialize job control list
    init_job_control_funcs();

    debug = 0; // initialize global variable debug to false;

    // First, check for command line arguments
    // If, -d is given, debug is set to true when this function is called.
    check_options(argc, argv); 

    // copy envp to dynamic sorted list
    envp320 = initialize_envp(envp); // free this when finished
    new_map_size = 0;

    //indicated if the user pressed backspace last
    has_backspaced = 0;
    just_paged = 0;
    should_execute_after_page = 0;

    // the path will go between promptHead and promptTail
    promptHead = CYAN"[";
    promptTail = "]"CYAN NOCOLOR" "BRED"320sh> "NOCOLOR;
    startPtr = NULL;
    pwd = NULL; // the working directory that will be called at every iteration.

    last_line = (char *)malloc(MAX_INPUT);  // used to store last line read 
    last_line_poz = 0;                        // used to store place in last_line array

    // what to do at exit
    atexit(house_keeping);

}

void main_loop()
{
    int finished = 0;         // determines when to end loop
    // main loop to read input
    char *cursor;           // input commands 
    char last_char;
    int rv;
    int count;
    while (!finished) 
    {
        //int chnaged_has_backspaced_to_false = 0;
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
                    // STDOUT("KILLING ID ");
                    // char conversion_buff[MAX_INT_CONVERSION_LENGTH];
                    // sprintf(conversion_buff, "%d", job_ids[i]);
                    // STDOUT(conversion_buff);
                    // STDOUT("\n");

                    kill_job(NULL, job_ids[i]);
                }
                
                i++;
            }
         }

        // Set the path to the working directory.
        char cwd[MAX_PROMPT];
        getcwd(cwd, MAX_PROMPT);
        pwd = cwd;

        // Print the prompt to stdout
        if(!has_backspaced)
        {
            //STDOUT("print prompt");
            STDOUT(promptHead);
            rv = STDOUT(pwd);
            STDOUT(promptTail);

            //leave this here- this is the proper history showing
            if(just_paged == 1)
               STDOUT(old_text);
            
        }
        else
        {
            has_backspaced = 0;
            just_paged = 0;
        }
    

        if (!rv) 
        { 
          // end loop
          finished = 1;
          break;
        }

        // read and parse the input
        rv = 1;
        count = 0;
        if(!has_backspaced)
        {
            cursor = cmd;
        }
        
        if(startPtr == NULL)
            startPtr = cursor;

        last_char = 1;
        
        if(has_backspaced)
        {
            int i = 0;
            while(last_line[i] != '\0')
            {
                last_char = last_line[i];
            }
        }

        for(;   
          rv && (++count < (MAX_INPUT-1)) && (last_char != '\n') && !has_backspaced;
          cursor++) 
        { 
            // keep looping until last_char is a newline
            // end loop is nothing is read.
            rv = Read(0, cursor, 1);
            last_char = *cursor;

            //store last line
            if(last_line_poz < MAX_INPUT - 1 && !has_backspaced)
            {
                last_line[last_line_poz] = last_char;
                last_line_poz++;
            }

            // TODO: check for special chars here
            // Ctrl+C       03
            // Backspace    08
            // Tab          09
            // Up    \[A    27 91 65
            // Down  \[B    27 91 66
            // Right \[C    27 91 67
            // Left  \[D    27 91 68
            // switch statement that respond appropriately
            // 
            // True if special command succeeds (command is special)
            has_backspaced = 0;
            if (execute_special_command(cursor , startPtr , last_line, last_line_poz))
                break;  
            else
            {
                //has not paged and did not page up or down
                just_paged = 0;
                has_pressed_page_up = 0;
            }
            
            // This is for when the program is launched with ./launcher
            //if we just paged up or down, we dont need to newline. Check to see if the next one is newline 
            int is_newline = 0;
            if(last_char == '\n')
                is_newline = 1;

            char last_char_as_string[2] = {last_char, '\0'};
            if(just_paged == 1)
            {
                if(is_newline != 1)
                   STDOUT(last_char_as_string);
            }
            else
                STDOUT(last_char_as_string);
        } 
        if(!has_backspaced)
        {
            *cursor = '\0';

            if (!rv) 
            { 
                // end loop
                finished = 1;
                break;
            }

            // Execute the command, handling built-in commands separately 
            // Just echo the command line for now
            // write(1, cmd, strnlen(cmd, MAX_INPUT));
            //if there are only two arguments check to see if a shell script was given
            if(!has_backspaced && !pressed_up_or_down)
            {

                strcat(old_text, cmd);
                strcat(old_text, "\0");

                strcpy(cmd, old_text); // copy over whole command back into cmd
                strcpy(whole_cmd, old_text); // this is to save in history

                // there can be pipes
                int num_commands = 1; // number of pipes + 1
                char *all_commands[MAX_PIPES]; // if there are pipes, this will hold each pipe delimited string
                parse_pipes(cmd, all_commands, &num_commands);

                pipes_left = num_commands;
                if (pipes_left > 1)
                    piping = true;
                else
                    piping = false;

                int i = 0;
                for (; i < num_commands; i++)
                {
                    if (i == 0)
                        is_first_command = true;
                    else
                        is_first_command = false;

                    strcpy(old_text, all_commands[i]); // time to execute each command in list
                    strcat(old_text, "\0");

                    reset_redirections();

                    pipes_left--;

                    // execute?
                    bool execute = true;

                    // Set stdin redirect fd if there is a '<' symbol
                    // This returns false if there is an error (file does not exist or is a dir.)
                    if (!set_stdin_file(old_text)) 
                        execute = false;

                    // Set any output redirects, including stdout, stderr, or any other
                    // if there is(are) '>' symbol(s).
                    // This returns false if there is an error (file is a dir.)
                    if (!set_out_files(old_text))
                        execute = false;

                    // this function changes multiple spaces to a single space and
                    // makes sure commands do not end in a space
                    trim_and_move_redundant_spaces(old_text);

                    // STDOUT("Executing command\n");
                    if (execute)
                        execute_command(old_text, envp320, 0);
                    // STDOUT("returned from execute\n");

                    if(should_execute_after_page == 0)
                        strcpy(old_text, "");
                    else
                        should_execute_after_page = 0;
                }
            }
            pressed_up_or_down = false;
            //just_paged = 1;

        }
    } // end of main loop
}

void house_keeping()
{
    int i = 0;
    for (; i < new_map_size; i++)
    {
        free(new_var_mapping[i]);
    }
    free(envp320);
    //fclose(ofp);
}