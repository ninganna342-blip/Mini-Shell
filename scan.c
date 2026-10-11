#include "main.h"

Slist *head = NULL;

pid_t pid;

char current_command[50];

void scan_input(char *prompt, char *input_string)
{
    // Register signal handlers for the shell
    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);
    signal(SIGCHLD, signal_handler);

    while(1)
    {
        pid = 0;

        // Display the shell prompt in green
        printf(ANSI_COLOR_MAGENTA "%s " ANSI_COLOR_RESET, prompt);

        // Read command from the user
        scanf(" %[^\n]", input_string);

        strcpy(current_command, input_string);

        // Handle PS1 prompt modification
        if(strncmp(input_string, "PS1", 3) == 0)
        {
            if(input_string[3] != '=')
            {
                printf("PS1 : command not found\n");
                continue;
            }

            if(input_string[4] == '\0')
            {
                printf("PS1 : command not found\n");
                continue;
            }

            char *ptr = input_string + 4;

            // Check whether spaces are present in the PS1 value
            while(*ptr != '\0')
            {
                if(*ptr == ' ')
                {
                    printf("PS1 : command not found\n");
                    break;
                }

                ptr++;
            }

            if(*ptr == '\0')
            {
                strcpy(prompt, input_string + 4);
            }

            continue;
        }

        // Check whether the command contains a pipe
        if(strchr(input_string, '|') != NULL)
        {
            execute_pipeline(input_string);
            continue;
        }

        // Extract the first word to identify the command type
        char *command = get_command(input_string);

        int ret = check_command_type(command);

        if(ret == BUILTIN)
        {
            // Execute shell built-in command
            execute_internal_cmd(input_string);
        }

        else if(ret == EXTERNAL)
        {
            // Execute external command using fork and exec
            execute_external_cmd(input_string);
        }

        else
        {
            // Display error for an unknown command
            printf("%s : command not found\n", input_string);
        }
    }
}


char *get_command(char *input_string)
{
    // Extract the first word from the input string
    static char command[50];

    int i = 0;

    while(*input_string != ' ' && *input_string != '\n' && *input_string != '\0')
    {
        command[i++] = *input_string;

        input_string++;
    }

    command[i] = '\0';

    return command;
}


void execute_internal_cmd(char *input_string)
{
    // Handle exit command
    if(strcmp(input_string, "exit") == 0)
    {
        exit(0);
    }

    // Handle pwd command
    else if(strcmp(input_string, "pwd") == 0)
    {
        char str[100];

        getcwd(str, 100);

        printf("%s\n", str);
    }

    // Handle cd command
    else if(strncmp(input_string, "cd", 2) == 0 &&(input_string[2] == ' ' || input_string[2] == '\0'))
    {
        char *arg = input_string + 2;

        // Skip spaces after cd
        while(*arg == ' ')
        {
            arg++;
        }

        if(*arg == '\0')
        {
            printf("cd: missing operand\n");
        }

        else
        {
            // Change the current working directory
            if(chdir(arg) == -1)
            {
                perror("cd");
            }
        }
    }

    // Handle echo command
    else if(strncmp(input_string, "echo", 4) == 0 &&(input_string[4] == ' ' || input_string[4] == '\0'))
    {
        char *input = input_string + 4;

        // Skip spaces after echo
        while(*input == ' ')
        {
            input++;
        }

        // Display the SHELL environment variable
        if(strcmp(input, "$SHELL") == 0)
        {
            printf("%s\n", getenv("SHELL"));
        }

        // Display the exit status of the previous command
        else if(strcmp(input, "$?") == 0)
        {
            printf("%d\n", status);
        }

        // Display the PID of the shell
        else if(strcmp(input, "$$") == 0)
        {
            printf("%d\n", getpid());
        }

        else
        {
            if(*input == '\0')
            {
                printf("\n");
            }

            else
            {
                printf("%s\n", input);
            }
        }
    }

    // Display currently stored jobs
    else if(strcmp(input_string, "jobs") == 0)
    {
        // Traverse the linked list and print the jobs
        Slist *temp = head;

        if(temp == NULL)
        {
            return;
        }

        while(temp)
        {
            printf("%d %s\n", temp->pid, temp->process);

            temp = temp->link;
        }
    }

    // Continue the most recent stopped job in background
    else if(strcmp(input_string, "bg") == 0)
    {
        if(head == NULL)
        {
            printf("bg: no current job\n");
        }

        else
        {
            kill(head->pid, SIGCONT);
        }
    }

    // Continue the most recent stopped job in foreground
    else if(strcmp(input_string, "fg") == 0)
    {
        if(head == NULL)
        {
            printf("fg: no current job\n");
        }

        else
        {
            pid_t child_pid = head->pid;

            // Resume the stopped process
            kill(child_pid, SIGCONT);

            // Wait until the foreground process exits or stops
            waitpid(child_pid, &status, WUNTRACED);

            // Remove the job if the process has exited
            if(WIFEXITED(status) || WIFSIGNALED(status))
            {
                delete_first(child_pid);
            }
        }
    }
}


void signal_handler(int sig_num)
{
    // Handle Ctrl+C
    if(sig_num == SIGINT)
    {
        if(pid == 0)
        {
            printf(ANSI_COLOR_MAGENTA "\n%s " ANSI_COLOR_RESET, prompt);
            fflush(stdout);
        }
    }

    // Handle Ctrl+Z
    if(sig_num == SIGTSTP)
    {
        if(pid == 0)
        {
            printf(ANSI_COLOR_MAGENTA "\n%s " ANSI_COLOR_RESET, prompt);
            fflush(stdout);
        }
    }

    // Handle termination of child processes
    if(sig_num == SIGCHLD)
    {
        pid_t child_pid;

        // Check for completed background children without blocking
        while((child_pid = waitpid(-1, &status, WNOHANG)) > 0)
        {
            // Remove completed job from the linked list
            if(WIFEXITED(status))
            {
                delete_first(child_pid);
            }
        }
    }
}


void execute_external_cmd(char *input_string)
{
    char *argv[20];

    int i = 0;

    // Split the command into arguments
    char *token = strtok(input_string, " ");

    while(token != NULL)
    {
        argv[i++] = token;

        token = strtok(NULL, " ");
    }

    argv[i] = NULL;

    // Create a child process
    pid = fork();

    if(pid > 0)
    {
        // Parent waits for the foreground child
        waitpid(pid, &status, WUNTRACED);

        // Add stopped process to the job list
        if(WIFSTOPPED(status))
        {
            insert_first(pid, current_command);

            printf("\n");
        }
    }

    else if(pid == 0)
    {
        // Restore default signal handling in the child
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        // Replace child process with the requested command
        execvp(argv[0], argv);

        // Execute only if execvp fails
        perror("execvp");

        exit(1);
    }

    else
    {
        // Handle fork failure
        perror("fork");
    }
}


void execute_pipeline(char *input_string)
{
    char *commands[20];

    int command_count = 0;

    // Split the input into individual commands using pipe
    char *token = strtok(input_string, "|");

    while(token != NULL)
    {
        // Remove leading spaces from each command
        while(*token == ' ')
        {
            token++;
        }

        commands[command_count++] = token;

        token = strtok(NULL, "|");
    }

    // Create pipes between the commands
    int fd[command_count - 1][2];

    for(int i = 0; i < command_count - 1; i++)
    {
        if(pipe(fd[i]) == -1)
        {
            perror("pipe");
            return;
        }
    }

    // Create one child process for each command
    for(int i = 0; i < command_count; i++)
    {
        pid_t pid = fork();

        if(pid == -1)
        {
            perror("fork");
            return;
        }

        if(pid == 0)
        {
            // Connect input from the previous pipe
            if(i > 0)
            {
                dup2(fd[i - 1][0], STDIN_FILENO);
            }

            // Connect output to the next pipe
            if(i < command_count - 1)
            {
                dup2(fd[i][1], STDOUT_FILENO);
            }

            // Close all pipe file descriptors after dup2
            for(int j = 0; j < command_count - 1; j++)
            {
                close(fd[j][0]);
                close(fd[j][1]);
            }

            char *argv[20];

            int arg_count = 0;

            // Split the individual command into arguments
            char *arg = strtok(commands[i], " ");

            while(arg != NULL)
            {
                argv[arg_count++] = arg;

                arg = strtok(NULL, " ");
            }

            argv[arg_count] = NULL;

            // Execute the command
            execvp(argv[0], argv);

            // Execute only if execvp fails
            perror(argv[0]);

            exit(1);
        }
    }

    // Parent closes all pipe descriptors
    for(int i = 0; i < command_count - 1; i++)
    {
        close(fd[i][0]);
        close(fd[i][1]);
    }

    // Wait for all pipeline child processes
    for(int i = 0; i < command_count; i++)
    {
        wait(&status);
    }
}


int insert_first(int pid, char *process)
{
    // Allocate memory for a new job node
    Slist *new = malloc(sizeof(Slist));

    if(new == NULL)
    {
        return 1;
    }

    new->pid = pid;

    strcpy(new->process, process);

    new->link = NULL;

    if(head == NULL)
    {
        head = new;

        return 0;
    }

    // Insert the new job at the beginning of the list
    new->link = head;

    head = new;

    return 0;
}


int delete_first(pid_t child_pid)
{
    Slist *temp = head;

    Slist *prev = NULL;

    // Search the linked list for the given PID
    while(temp != NULL)
    {
        if(temp->pid == child_pid)
        {
            // Delete the first node
            if(prev == NULL)
            {
                head = temp->link;
            }

            // Delete a node from the middle or end
            else
            {
                prev->link = temp->link;
            }

            free(temp);

            return 0;
        }

        prev = temp;

        temp = temp->link;
    }

    return 1;
}
