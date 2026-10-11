#include "main.h"

int check_command_type(char *command)
{
   // List of shell built-in commands
   char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
                       "set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
                       "exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help",
                       "jobs", "bg", "fg", NULL};

   // Check whether the command is a built-in command
   for(int i=0; builtins[i]; i++)
   {
       if(strcmp(command, builtins[i]) == 0)
       {
          return BUILTIN;
       }
   }


   // Check whether the command exists in the external command list
   for(int i=0; external_commands[i]; i++)
   {
       if(strcmp(command, external_commands[i]) == 0)
       {
           return EXTERNAL;
       }
   }

   // Command is neither built-in nor external
   return NO_COMMAND;
}


void extract_external_commands(char **external_commands)
{
   // Open the file containing the list of external commands
   int fd = open("External_cmd.txt", O_RDONLY);

   if(fd == -1)
   {
      perror("open");
      return;
   }

   ssize_t n;
   char ch;
   int i;
   int k = 0;

   while(1)
   {
      char *command = NULL;
      i = 0;

      // Read one character at a time until newline or end of file
      while((n = read(fd, &ch, 1)) > 0)
      {
        if(ch == '\r')
        {
            continue;
        }

        if(ch == '\n')
        {
          break;
        }

        // Increase memory to store the current command
        command = realloc(command, i + 1);

        if(command == NULL)
        {
            perror("realloc");
            close(fd);
            return;
        }

        command[i] = ch;
        i++;
      }

      // Stop when end of file is reached without any remaining data
      if(n == 0 && i == 0)
      {
        break;
      }

      // Allocate space for the null terminator
      command = realloc(command, i + 1);

      if(command == NULL)
      {
        perror("realloc");
        close(fd);
        return;
      }

      command[i] = '\0';

      // Store the command in the external command list
      external_commands[k] = command;
      k++;

      if(n == 0)
      {
         break;
      }
   }

   // Mark the end of the external command list
   external_commands[k] = NULL;

   close(fd);
}
