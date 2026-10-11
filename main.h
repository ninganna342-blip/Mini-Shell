#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// Command type identifiers
#define BUILTIN     1
#define EXTERNAL    2
#define NO_COMMAND  3

// ANSI color codes used for shell messages and prompt
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Linked list node for maintaining background/stopped jobs
typedef struct list
{
    int pid;
    char process[50];
    struct list *link;
} Slist;

extern char prompt[50];
extern int status;
extern char **external_commands;

// Input and command processing functions
void scan_input(char *prompt, char *input_string);
char *get_command(char *input_string);

// Command execution functions for external
void execute_external_cmd(char *input_string);
void execute_pipeline(char *input_string);

// Command execution functions for internal
void execute_internal_cmd(char *input_string);
void echo(char *input_string, int status);

// Job control functions
int insert_first(int pid, char *process);
int delete_first(pid_t child_pid);

// Prompt and command validation functions
void copy_change(char *prompt, char *input_string);
int check_command_type(char *command);

// Signal handling
void signal_handler(int sig_num);

// Read external commands from the command file
void extract_external_commands(char **external_commands);

#endif
