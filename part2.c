#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "part1.h"
#include "part2.h"
#include "utils.h"

void parse_pipes(char *cmd, char *all_commands[], int *num_commands)
{
    // set num_commands to number of commands
    // use strtok to get all commands 

    char *tok = strtok(cmd, "|");
    int i = 0;
    while (tok)
    {
        // make sure tok doesn't start on a space
        while (tok[0] == ' ')
            tok++;

        all_commands[i] = tok;

        tok = strtok(NULL, "|");
        i++;
    }
    *num_commands = i;

    //test
    // int j = 0;
    // for (; j < *num_commands; j++)
    // {
    //     print_stdout("\n\n", all_commands[j], "\n\n", 0);
    // }


}

/**
 *  Aquire file names after a < symbol. If file does not exist or
 *  is a directory, return false.
 *
 * If there is a > symbol, set it to the global variable.
 */
bool set_stdin_file(char *full_command)
{
    // if there is a '<', the file name is the content between the symbol starting at 
    // the first nonspace and nonzero until the first space or zero.


    char* addr_of_redirect_stdin = (char*) strstr(full_command, "<");

    if (!addr_of_redirect_stdin) // return true bc there is no error.
        return true;

    // acquire name of file
    char file_name[100];
    int index;

    // set index to start of file name
    index = addr_of_redirect_stdin - full_command + 1;
    while (full_command[index] == ' ')
        index++;

    // copy over file name
    int i = 0;
    while (full_command[index] != '\n' && full_command[index] != ' ' && full_command[index] != '|' && full_command[index] != '>' && full_command[index] != '\0' && full_command[index] != '<')
    {
        file_name[i] = full_command[index];
        index++;
        i++;
    }
    file_name[i] = 0;

    // print_stdout(file_name, "\n", 0);    
    
    // if file name is blank
    if (file_name[0] == '\0' || file_name[0] == '\n' )
    {
        print_stderr("320sh: syntax error near unexpected token `newline\'\n", 0);
        return false;
    }

    // use STAT to check if the file exists, if not, let the user know and restart this loop.
    if (file_exists_and_is_not_dir(file_name))
    {
        // we good!
        // sets file name to global var and checks if it exists and is not a dir
        // if it exists, mark it as existing so we can redirect stdin        
        stdin_redirect_fd = Open(file_name, O_RDONLY);
        redirect_stdin = true;

        // Replace  < file_name with spaces
        index = addr_of_redirect_stdin - full_command; // index of '<'
        full_command[index] = ' ';

        while (full_command[index] != '\n' && full_command[index] != '|' && full_command[index] != '>' && full_command[index] != '\0' && full_command[index] != '<')
        {
            full_command[index] = ' ';
            index++;
        }

        return true;
    }

    reset_redirections();
    return false;

}

/**
 * Aquire file names after any > symbols. If any file is a directory,
 * return false.
 *
 * If there are < symbols, set them to the global variable(s).
 */
bool set_out_files(char *full_command)
{
    // then, any > symbols
    // if there is a '>', the file name is the content between the symbol starting at
    // the first nonspace and nonzero until the first space or zero.
    // if the file does not exist, create it.

    char *addr_of_redirect_stdout = (char*) strstr(full_command, ">");

    if (!addr_of_redirect_stdout) // return true bc there is no error.
        return true;

    // loop and acquire name of files
    while (addr_of_redirect_stdout)
    {

        // acquire name of file
        char file_name[100];
        int index;
        int descriptor = 1;
        bool not_stdout = false;

        // set index to start of file name
        index = addr_of_redirect_stdout - full_command + 1;

        // Check for #>
        int index_of_digit = 0;
        if (isdigit(full_command[index - 2]))
        {
            // check to make sure it's not something like asfd4>
            // also check for more than one digit

            int i = index - 3;
            while (isdigit(full_command[i]))
            {
                i--;
            }

            if (full_command[i] == ' ')
            {
                // good.
                i++;
                index_of_digit = i;
                char num[8];
                int j = 0;
                while (full_command[i] != '>')
                {
                    num[j++] = full_command[i++];
                }

                num[j] = 0;

                // test
                // print_stdout("\n", num, "\n", 0);

                descriptor = atoi(num);
                not_stdout = true;
            }
        }

        while (full_command[index] == ' ')
            index++;

        // copy over file name
        int i = 0;
        while (full_command[index] != '\n' && full_command[index] != ' ' && full_command[index] != '|' && full_command[index] != '>' && full_command[index] != '\0' && full_command[index] != '<')
        {
            file_name[i] = full_command[index];
            index++;
            i++;
        }
        file_name[i] = 0;

        // check if stdout, stderr, or other
        // print_stdout("\n", file_name, "\n", 0);

        // if file name is blank
        if (file_name[0] == '\0' || file_name[0] == '\n' )
        {
            print_stderr("320sh: syntax error near unexpected token `newline\'\n", 0);
            return false;
        }

        // sets file name to global var and checks if it is not a dir
        // if yes, mark it as existing so we can redirect stdin

        if (file_is_not_dir(file_name))
        {
            // open descriptor
            // delete file before creating
            remove(file_name);
            
            otherout_redirect_fd[num_other_redirects] = Open(file_name, O_WRONLY | O_CREAT);
            descriptors_to_replace[num_other_redirects] = descriptor;

            num_other_redirects++;

            // Replace  > file_name with spaces
            if (!not_stdout)
            {
                index = addr_of_redirect_stdout - full_command; // index of '<'
            }
            else // not 1
            {
                index = index_of_digit;
                while (full_command[index] != '>')
                {
                    full_command[index] = ' ';
                    index++;
                }
            }
            full_command[index] = ' ';

            while (full_command[index] != '\n' && full_command[index] != '|' && full_command[index] != '>' && full_command[index] != '\0' && full_command[index] != '<')
            {
                full_command[index] = ' ';
                index++;
            }
        }
        else
        {
            reset_redirections();
            return false;
        }

        // next!
        char *next = addr_of_redirect_stdout + 1;
        addr_of_redirect_stdout = (char*) strstr(next, ">");
    }
    
    return true;
}


// Useful for stdin
bool file_exists_and_is_not_dir(char *file)
{
    // use stat to check if a file exists and is not a directory
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
            return false;
        }

        // Return true whether or not the "file" we found is a file or dir
        return true;
    }

    print_stderr("320sh: ", file, ": No such file or directory\n", 0);
    return false;
}

// Useful for output
bool file_is_not_dir(char *file)
{
    // use stat to check if a file is not a directory
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
            return false;
        }

        // Return true whether or not the "file" we found is a file or dir
        return true;
    }

    return true;
}

void reset_redirections()
{
    if (redirect_stdin)
        Close(stdin_redirect_fd);
    if (num_other_redirects > 0)
    {
        // close all other redirects
        int i = 0;
        for (; i < num_other_redirects; i++)
            Close(otherout_redirect_fd[i]);
    }

    redirect_stdin = false;
    num_other_redirects = 0;
}

void change_redirect_descriptors()
{
    // Use Dup2 to change file descriptors
    if (redirect_stdin)
    {
        Dup2(stdin_redirect_fd, 0);
        Close(stdin_redirect_fd);
    }

    if (num_other_redirects > 0)
    {
        int i = 0;
        for (; i < num_other_redirects; i++)
        {
            Dup2(otherout_redirect_fd[i], descriptors_to_replace[i]);
            Close(otherout_redirect_fd[i]);
        }
    }
}

void change_piping_descriptors()
{
    if (piping)
    {
        if (is_first_command)
        {
            // first command
            // pipe stdin out to next command
            int fd = Open("ABC123XYZ.txt", O_WRONLY | O_CREAT);
            Dup2(fd, 1);
            Close(fd);
        }
        else if (pipes_left == 0)
        {
            // last command
            // pipe stdout in from prev command
            // Close(prev_pipefd[1]);   
            int fd = Open("ABC123XYZ.txt", O_RDONLY);
            Dup2(fd, 0);
            Close(fd);

            remove("ABC123XYZ.txt");

        }
        else
        {
            // not first or last
            // pipe stdout in from prev command
            int fd = Open("ABC123XYZ.txt", O_RDONLY);
            Dup2(fd, 0);
            Close(fd);

            remove("ABC123XYZ.txt");

            // pipe stdin out to next command
            fd = Open("ABC123XYZ.txt", O_WRONLY | O_CREAT);
            Dup2(fd, 1);
            Close(fd);
        }
    }
}

int execute_history()
{
	// create concatenated string representing $HOME/.320sh_history
    char history_path[sizeof(get_binding("HOME")) + sizeof(HISTORY) + 2];
    sprintf(history_path, "%s/%s", get_binding("HOME"), HISTORY);

    // open $HOME/.320sh_history
    FILE*ofp = fopen(history_path, "r");

    //we now have to just print out to command line history list
    //read file line by line and print out cmmand history number, and the command
    char line[MAX_LINE_SIZE];
    int history_num = 1;
    char num[15];
    while(fgets(line, sizeof(line), ofp) != NULL)
    {
    	sprintf(num, "%d" , history_num);
    	STDOUT(num);
    	STDOUT(" ");
    	STDOUT(line);
    	history_num++;
    }

    // close $HOME/.320sh_history
    fclose(ofp);
    return EXIT_SUCCESS;
}

int clear_history()
{
	// create concatenated string representing $HOME/.320sh_history
    char history_path[sizeof(get_binding("HOME")) + sizeof(HISTORY) + 2];
    sprintf(history_path, "%s/%s", get_binding("HOME"), HISTORY);

    // open $HOME/.320sh_history
    FILE*ofp = fopen(history_path, "w");

    // close $HOME/.320sh_history
    fclose(ofp);
    return EXIT_SUCCESS;
}


int page_up()
{
    // STDOUT("paged up\n");
    just_paged = 1;
    should_execute_after_page = 1;
    if(has_pressed_page_up == 0)
    {
        history_stack = init_stack();
        cursor_stack = init_stack();

        // create concatenated string representing $HOME/.320sh_history
        char history_path[sizeof(get_binding("HOME")) + sizeof(HISTORY) + 2];
        sprintf(history_path, "%s/%s", get_binding("HOME"), HISTORY);

        // open $HOME/.320sh_history
        page_search_file_pointer = fopen(history_path, "r");

        //go to the end 
        int read = 0;
        char * line = (char *) malloc(MAX_DIRECTORY_LENGTH);
        size_t len = MAX_DIRECTORY_LENGTH;
        while ((read = getline(&line, &len, page_search_file_pointer)) != -1)
        {
            push(line, history_stack);
        }
        
        //close $HOME/.320sh_history
        fclose(page_search_file_pointer);
    }

    if(has_pressed_page_up == 0)
        has_pressed_page_up = 1;
    
    //if stack is empty, we can't go through the commands
    if(peek(history_stack) == NULL)
        return EXIT_SUCCESS;
    
    //if pressed paged up - pop off of history stack and and put onto cursor
    char *command = pop(history_stack);
    if(command == NULL)
        return EXIT_SUCCESS;
    push(command, cursor_stack);
    
    //if the command has a newline remove it
    char *ptr = command;
    while(*ptr != '\0')
    {
        if(*ptr == '\n')
            *ptr = '\0';

        ptr++;
    }
    
    //save old command to buffer to execute later
//    STDOUT("copying into old text 404 part2.c\n");
   // STDOUT(command);
    //STDOUT(" is command\n");
    strcpy(old_text, command);

    pressed_up_or_down = true;

    return EXIT_SUCCESS;
}


int page_down()
{
    // STDOUT("paged down\n");
    just_paged = 1;
    should_execute_after_page = 1;
    if(has_pressed_page_up == 1)
    {
        if(peek(cursor_stack) == NULL)
            return EXIT_SUCCESS;
        
        char *command = pop(cursor_stack);
        if(command == NULL)
            return EXIT_SUCCESS;
        
        push(command, history_stack);

        command = peek(cursor_stack);
        if(command == NULL)
            return EXIT_SUCCESS;
        
        //if the command has a newline remove it
        char *ptr = command;
        while(*ptr != '\0')
        {
            if(*ptr == '\n')
                *ptr = '\0';

            ptr++;
        }
        //save old command to buffer to execute later
//        STDOUT("copying into old text at 440 part.c\n");
        strcpy(old_text, command);
    }

    pressed_up_or_down = true;

    return EXIT_SUCCESS;
}