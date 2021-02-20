// ACADEMIC INTEGRITY PLEDGE
//
// - I have not used source code obtained from another student nor
//   any other unauthorized source, either modified or unmodified.
//
// - All source code and documentation used in my program is either
//   my original work or was derived by me from the source code
//   published in the textbook for this course or presented in
//   class.
//
// - I have not discussed coding details about this project with
//   anyone other than my instructor. I understand that I may discuss
//   the concepts of this program with other students and that another
//   student may help me debug my program so long as neither of us
//   writes anything during the discussion or modifies any computer
//   file during the discussion.
//
// - I have violated neither the spirit nor letter of these restrictions.
//
//
//
// Signed:Keith Ratchford (kdr60)  Date:2/14/2021

// 3460:426 Lab 1 - Basic C shell rev. 9/10/2020

/* Basic shell */

/*
 * This is a very minimal shell. It finds an executable in the
 * PATH, then loads it and executes it (using execv). Since
 * it uses "." (dot) as a separator, it cannot handle file
 * names like "minishell.h"
 *
 * The focus on this exercise is to use fork, PATH variables,
 * and execv. 
 */

#include <ctype.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>//use standard booleans

#define MAX_ARGS		64
#define MAX_ARG_LEN		16
#define MAX_LINE_LEN	80
#define WHITESPACE		" ,\t\n"

struct command_t {
   char *name;
   int argc;
   char *argv[MAX_ARGS];
};

/* Function prototypes */
int parseCommand(char *, struct command_t *);
void printPrompt();
void readCommand(char *);

int main(int argc, char *argv[]) {
   int pid;
   int status;
   char cmdLine[MAX_LINE_LEN];
   struct command_t command;

   while(true) {
      printPrompt();
      /* Read the command line and parse it */
      readCommand(cmdLine);
      parseCommand(cmdLine, &command);
      command.argv[command.argc] = NULL;

      //check for no command entered
      if(strlen(command.name) == 0){
          //specify noop
	  command.name = "noop";
      }
      //test for shortcuts
      if(!strcmp(command.name, "H")){//prinnt help menu
          char *help = "\nName\n"
			   "\tkdr60Shell - basic C shell with several shortcuts for basic file manipulation and some other miscellaneous operations\n"
                        "\nDescription\n"
                           "\tkdr60Shell is a very basic command line interpreter which can parse linux commands as well as some \'shortcut commands\' which are listed as follows\n"
                        "\nShortcut Commands\n"
                           "\t - C file1 file2: copies the content of file1 to file2\n"
                           "\t - D file1: deletes file1 from the directory\n"
                           "\t - E comment: prints comment back to the screen\n"
                           "\t - H: displays this help manual\n"
                           "\t - L: displays the working directory and all files in the current directory\n"
                           "\t - M file1: opens the nano text editor to create a new file named \'file1\'\n"
                           "\t - P file1: displays the content of file1 to the screen\n"
                           "\t - Q: quit out of kdr60Shell\n"
                           "\t - S: opens an instance of Firefox web-browser\n"
                           "\t - W: clears console screen\n"
                        "\nBasic Usage\n"
                           "\tEnter either a shortcut command provided above or another linux command and press the enter key to execute the command.\n\n";
          printf("%s ", help);
          command.name = "noop";
      }else if(!strcmp(command.name, "S")){//web browser shortcut
          command.name = "firefox";
          command.argv[0] = "firefox";
      }else if(!strcmp(command.name, "E")){//echo shortcut
          command.name = "echo";
	  command.argv[0] = "echo";
      }else if(!strcmp(command.name, "L")){//list shortcut
          printf("\n");
          if ((pid = fork()) == 0) {//create child to run PWD
              command.name = "pwd";
	      command.argv[0] = "pwd";
              execvp(command.name, command.argv);
	      /* handle exec error */
	      perror("Error in exec - terminating the child");
	      return(0);
          }else if(pid < 0){
              perror("Error during fork creation");
              return -1;
          }
          //wait for pwd
          wait(&status);
          //skip line
	  printf("\n");
          //set command structure to execute "ls -l" call
	  command.name = "ls";
          command.argv[0] = "ls";
          command.argv[1] = "-l";
          command.argv[2] = NULL;
	}else if(!strcmp(command.name, "W")){//clear shortcut
	      command.name = "clear";
	      command.argv[0] = "clear";
	}else if(!strcmp(command.name, "P")){//Display file shortcut
              command.name = "more";
	      command.argv[0] = "more";
        }else if(!strcmp(command.name, "C")){//Copy file shortcut
              command.name = "cp";
	      command.argv[0] = "cp";
	}else if(!strcmp(command.name, "M")){//make file shortcut
	      command.name = "nano";
	      command.argv[0] = "nano";
        }else if(!strcmp(command.name, "D")){// delete file shortcut
              command.name = "rm";
	      command.argv[0] = "rm";
	}else if(!strcmp(command.name, "Q")){
	  /* exit command entered so terminate the shell */
	  break;
	}

      //check for noop
      if(strcmp(command.name, "noop")){
          /* Create a child process to execute the command */
          if ((pid = fork()) == 0) {
             /* Child executing command */
             execvp(command.name, command.argv);
	     /* handle exec error */
	     perror("Error in exec - terminating the child");
	     return(0);
          }else if(pid < 0){
	    perror("Error during fork creation");
	    return -1;
          }
          /* Wait for the child to terminate for all shortcuts except for S */
         if(strcmp(command.name, "firefox")){
             wait(&status);
         }
      }
   }

   /* Shell termination */
   printf("\n\n shell: Terminating successfully\n");
   return 0;
}

/* End basic shell */

/* Parse Command function */

/* Determine command name and construct the parameter list.
 * This function will build argv[] and set the argc value.
 * argc is the number of "tokens" or words on the command line
 * argv[] is an array of strings (pointers to char *). The last
 * element in argv[] must be NULL. As we scan the command line
 * from the left, the first token goes in argv[0], the second in
 * argv[1], and so on. Each time we add a token to argv[],
 * we increment argc.
 */
int parseCommand(char *cLine, struct command_t *cmd) {
   int argc;
   char **clPtr;
   /* Initialization */
   clPtr = &cLine;	/* cLine is the command line */
   argc = 0;
   cmd->argv[argc] = (char *) malloc(MAX_ARG_LEN);
   /* Fill argv[] */
   while ((cmd->argv[argc] = strsep(clPtr, WHITESPACE)) != NULL) {
      cmd->argv[++argc] = (char *) malloc(MAX_ARG_LEN);
   }

   /* Set the command name and argc */
   cmd->argc = argc-1;
   cmd->name = (char *) malloc(sizeof(cmd->argv[0]));
   strcpy(cmd->name, cmd->argv[0]);
   return 1;
}

/* End parseCommand function */

/* Print prompt and read command functions - Nutt pp. 79-80 */

void printPrompt() {
   /* Build the prompt string to have the machine name,
    * current directory, or other desired information
    */
   char promptString[] = "linux(kdr60)|>\0";
   printf("%s ", promptString);
}

void readCommand(char *buffer) {
   /* This code uses any set of I/O functions, such as those in
    * the stdio library to read the entire command line into
    * the buffer. This implementation is greatly simplified,
    * but it does the job.
    */
   fgets(buffer, 80, stdin);
}

/* End printPrompt and readCommand */
