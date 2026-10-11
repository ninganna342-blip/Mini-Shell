/* Name: Ningannagouda
   Batch: INT26001B
   Title of the Project: Mini Shell
   Date: August 31, 2026
   Description: Developed a Linux-based Mini Shell in C supporting built-in and external commands. 
                Implemented process management using fork(), execvp(), and waitpid().

                Implemented command pipelines using pipe() and dup2() and signal handling using SIGINT, SIGTSTP, and SIGCHLD. 
                Added PS1 customization and basic job control using jobs, bg, and fg, with linked lists for process management.   */


#include "main.h"

// Store the shell prompt
char prompt[50] = "minishell$";

// Store the list of external commands
char **external_commands;

// Store the exit status of the last executed command
int status;

int main()
{
     // Clear the terminal screen
    system("clear");

    // Allocate memory for the external command list
    external_commands = calloc(100, sizeof(char *));

    if (external_commands == NULL)
    {
        perror("malloc");
        return 1;
    }

    // Read external commands from the command file
    extract_external_commands(external_commands);

    char input_string[100];

    // Start reading and executing user commands
    scan_input(prompt, input_string);

    // Free memory allocated for each external command
    for(int i = 0; external_commands[i] != NULL; i++)
    {
        free(external_commands[i]);
    }

    // Free the external command list
    free(external_commands);

    return 0;
}
