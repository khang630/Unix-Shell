#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h> //temporary??

#include <ctype.h>
#include "cd.h"
#include "path.h"
#include "history.h"
#include "alias.h"

#define MAX 512 //user's input is less than 512 bytes

void InteractiveMode();
void BatchMode(char *file);

int ParseCommands(char *userInput);		//e.g., "ls -a -l; who; date;" is converted to "ls -al" "who" "date"
int ParseCommandsPipe(char *full_line); //finding number of pipes in command line
void exec(char *command);
int ParseArgs(char *full_line, char *args[]); //e.g., "ls -a -l" is converted to "ls" "-a" "-l"
void ExecuteCommands(char *command, char *full_line);

void MyCD(char *dir_input, int arg_count);
void MyExit();
void MyPath(char *args[], int arg_count);

void CommandRedirect(char *args[], char *first_command, int arg_count, char *full_line);
//void PipeCommands(char *args[], char *first_command, int arg_count);
void PipeCommands(char *full_line);
void signalHandle(int sig);

char *trimWhiteSpace(char *line);
int isSpace(char *line);

char CURRENT_DIRECTORY[MAX]; //current directory
char *COMMANDS[MAX];		 //commands to be executed
char *MYHISTORY[20];		 //shell command history
char *MYALIAS[MAX];			 //stores alias name
char *MYALIASCOMMAND[MAX];	 //stores corresponding alias command
char *MYPATH;				 //my PATH variable
const char *ORIG_PATH_VAR;	 //The original PATH contents
bool EXIT_CALLED;			 //this is for the exit command (so the other commands run first)
pid_t parents_pid;

int main(int argc, char *argv[])
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);

	parents_pid = getpid();
	//initialize your shell's enviroment
	MYPATH = (char *)malloc(1024);
	if (MYPATH == NULL)
	{
		printf("Error allocating space for MYPATH\n");
		return 0;
	}
	memset(MYPATH, '\0', sizeof(MYPATH));
	ORIG_PATH_VAR = getenv("PATH"); // needs to include <stdlib.h>

	//save the original PATH, which is recovered on exit
	strcpy(MYPATH, ORIG_PATH_VAR);

	//make my own PATH, namely MYPATH
	setenv("MYPATH", MYPATH, 1);

	if (argc == 1)
		InteractiveMode();
	else if (argc == 2)
		BatchMode(argv[1]);
	else
	{
		printf("Invalid number of arguement. Exiting.\n");
		exit(1);
	}

	//Set path back to original
	setenv("MYPATH", ORIG_PATH_VAR, 1);

	//free all variables initialized by malloc()
	free(MYPATH); //free global variables

	return 0;
}

void BatchMode(char *file)
{
	int historycount = 0;

	printf("Entering Batch Mode...\n\n");
	//if history has been previously deleted, reset historycount
	//or if there is a function in history that is to be executed, execute it
	if (COMMANDS[0] != NULL)
	{
		if (strcmp(COMMANDS[0], "cleared") == 0)
		{
			historycount = 0;
			COMMANDS[0] = NULL;
		}
		else if (strcmp(COMMANDS[0], "historyexecute") == 0)
		{
			char *str = (char *)malloc(MAX);
			str = COMMANDS[1]; //give history -e command to str
			int cmd_num = ParseCommands(str);
			for (int i = 0; i < cmd_num; i++)
			{
				char *temp = strdup(COMMANDS[i]);
				temp = strtok(temp, " ");
				ExecuteCommands(temp, COMMANDS[i]);
			}
			free(str);
			COMMANDS[0] = NULL;
			COMMANDS[1] = NULL;
		}
	}

	FILE *fptr = fopen(file, "r");
	//error checking for fopen function
	if (fptr == NULL)
	{
		printf("Unable to open file. Exiting.\n");
		exit(1);
	}

	char *batch_command_line = (char *)malloc(MAX);
	if (batch_command_line == NULL)
	{
		printf("Error parsing command in BatchMode");
		fclose(fptr);
		return;
	}
	memset(batch_command_line, '\0', sizeof(batch_command_line));

	//reads a line from fptr, stores it into batch_command_line
	while (fgets(batch_command_line, MAX, fptr))
	{
		EXIT_CALLED = 0;
		//(1 point) Each line in the batch file is printed before being executed.
		printf("Executing the following command line: %s \n", batch_command_line);

		//while there are aliases stored, iterate through all to see if an alias matches
		for (int i = 0; i < MAX; ++i)
		{
			if (MYALIAS[i] == NULL)
			{
				break;
			}

			//remove newline from str bc fgets stores newline
			char *temp = strdup(batch_command_line);
			temp = strtok(temp, "\n");

			//compare to see if user command has an alias command
			if ((strcmp(temp, MYALIAS[i]) == 0))
			{
				//if there is a corresponding alias command, replace the current command with that
				batch_command_line = strdup(MYALIASCOMMAND[i]);
			}
		}

		//keeps track of entire line
		char *historytoken = strtok(batch_command_line, "\n"); //gets rid of endline so when history is called there is no space between histories
		MYHISTORY[historycount] = strdup(historytoken);
		historycount++;

		//parse batch_command_line to set the array COMMANDS[]
		//for example: COMMANDS[0]="ls -a -l", COMMANDS[1]="who", COMMANDS[2]="date"
		int cmd_count = ParseCommands(batch_command_line);

		//execute commands one by one
		for (int i = 0; i < cmd_count; i++)
		{
			char *temp = strdup(COMMANDS[i]); //for example: ls -a -l
			temp = strtok(temp, " ");		  //get the command
			ExecuteCommands(temp, COMMANDS[i]);
		}

		if (EXIT_CALLED)
			MyExit();
	}

	//free batch_command_line, and close fptr
	free(batch_command_line);
	fclose(fptr);
}

int ParseCommands(char *str)
{
	int i = 0;

	char *token = strtok(str, ";"); //breaks str into a series of tokens using ;

	while (token != NULL)
	{
		token = trimWhiteSpace(token);
		if (isSpace(token) != 0)
		{
			//error checking for possible bad user inputs
			//printf(" %s\n", token);
			//printf("error checking token\n"); //mytest

			//token = trimWhiteSpace(token); //trim white space off the ends of the command

			//save the current token into COMMANDS[]
			COMMANDS[i] = token;
			//printf("COMMAND[%d]: %s\n", i, COMMANDS[i]); //mytest

			//if you type last command with ;, the program adds an extra argument which is wrong.
			//so if I typed "ps a; ps x;", the program will say it has 3 arguments when there are only 2.
			//this if statement accounts for this so you can type "ps a; ps x" or "ps a; ps x;" and the arg count will still
			//be correct
			char *temp = COMMANDS[i];
			if (temp[0] != '\n')
			{
				i++;
			}
		}

		//move to the next token
		//passing on NULL as the argument tells strtok to keep going
		token = strtok(NULL, ";\n");
	}

	/*
	//if last command has no semi colon
	token = strtok(NULL, "\n");
	COMMANDS[i] = token;
	i++;
	*/

	char *token2 = strtok(str, "\n");

	//if there is only 1 command with no semi colon at the end. E.g "ps u"
	while (token != NULL)
	{
		//printf(" %s\n", token);

		token = trimWhiteSpace(token);
		if (isSpace(token) != 0)
		{
			//token = trimWhiteSpace(token);
			COMMANDS[i] = token;

			char *temp = COMMANDS[i];
			if (temp[0] != '\n')
			{
				i++;
			}
		}
	}

	//printf("Total # of commands: %d\n", i); //mytest
	return i;
}

void ExecuteCommands(char *command, char *full_line)
{
	//printf("successfully called ExecuteCommands function ------------------------------------------------ \n"); //mytest
	char *args[MAX]; //hold arguments

	//save full line for if an alias is used
	char *aliastemp = strdup(full_line);

	//parse full_line to get arguments and save them to args[] array
	int arg_count = ParseArgs(full_line, args);

	//printf("arg_count: %i\n", arg_count);
	//FOR SOME REASON arg_count RETURNS THE ARGUEMENT COUNT + 1
	//check if built-in function is called
	if (strcmp(command, "cd") == 0)
		MyCD(args[0], arg_count);

	else if (strcmp(command, "exit") == 0)
		EXIT_CALLED = 1;

	else if (strcmp(command, "path") == 0)
		MyPath(args, arg_count - 1);
	else if (strcmp(command, "myhistory") == 0)
		MyHistory(args, arg_count, MYHISTORY, COMMANDS);
	else if (strcmp(command, "alias") == 0)
		Alias(args, aliastemp, MYALIAS, MYALIASCOMMAND);
	else
		CommandRedirect(args, command, arg_count, full_line);

	//free memory used in ParsedArgs() function
	for (int i = 0; i < arg_count; i++)
	{
		if (args[i] != NULL)
		{
			args[i] = NULL;
		}
	}
}

int ParseArgs(char *full_line, char *args[])
{
	//printf("successfully called ParseArgs function ------------------------------------------------ \n"); //mytest
	int count = 0;
	int i = 0;

	//removes tabs and replaces with space
	while (full_line[i] != '\0')
	{
		if (isspace(full_line[i]))
		{
			full_line[i] = ' ';
		}
		i++;
	}

	//break full_line into a series of tokens by the delimiter space (or " ")
	char *token = strtok(full_line, " ");
	//skip over to the first argument

	//printf("We will skip this when iterating through arg* array later: %s\n", token); //mytest

	while (token != NULL)
	{
		//remove extra spaces

		//copy the current argument to args[] array
		args[count] = strdup(token);
		//printf("args[%d] has: %s\n", count, token); //mytest
		count++;
		//move to the next token (or argument)
		token = strtok(NULL, " ");
	}

	//printf("ParseArgument count: %d\n", count); //mytest

	return count + 1;
}

char *trimWhiteSpace(char *line)
{
	char *copy = (char *)malloc(MAX);
	if (copy == NULL)
	{
		printf("Error parsing command\n");
		return line;
	}
	int size = 0;

	//count the white spaces on the left of the command
	while (isspace(line[size]))
	{
		size++;
	}

	//copies everything past the white space into another string
	for (int i = 0; i < (int)strlen(line); i++)
	{
		copy[i] = line[size + i];
	}

	//place a new null terminator at the end of the copied string
	copy[size + strlen(line)] = '\0';

	//get the size of the copied string
	size = (int)strlen(copy);

	//get the white spaces on the right
	while (isspace(copy[size - 1]))
	{
		size--;
	}

	//copies everything but the right white space back into the original string
	for (int i = 0; i < size; i++)
	{
		line[i] = copy[i];
	}

	//pplace a new null terminator
	line[size] = '\0';

	//free the copied string
	free(copy);

	return line;
}

int isSpace(char *line)
{
	for (int i = 0; i < (int)strlen(line); i++)
	{
		if (!isspace(line[i]))
		{
			return 1;
		}
	}
	return 0;
}

void CommandRedirect(char *args[], char *first_command, int arg_count, char *full_line)
{
	//printf("CommandRedirect successfully called\n"); //mytest
	int status;
	pid_t pid;

	/*
	//if full_line contains "|", then the PipeCommands() is called
	PipeCommands(args, first_command, arg_count);
	*/

	//else excute the current command

	//set the new cmd[] array so that cmd[0] hold the actual command
	//cmd[1] - cmd[arg_count] hold the actual arguments
	//cmd[arg_count+1] hold the "NULL"
	char *cmd[arg_count + 1]; //add 1 to hold actual argument at cmd[0]. So if you have 2 args, do + 1 to account for the actual command at cmd[0]
	cmd[0] = first_command;

	//printf("Main command at cmd[0]: %s\n", cmd[0]); //mytest

	for (int i = 1; i < arg_count - 1; i++)
	{
		cmd[i] = args[i];
	}

	cmd[arg_count - 1] = NULL;

	pid = fork();
	if (pid == 0)
	{
		//setpgid(getpid(), getpid());

		//tcsetpgrp(0, getpgid(getpid()));

		signal(SIGINT, signalHandle);
		signal(SIGTSTP, signalHandle);

		execvp(cmd[0], cmd);
		printf("EXECVP FAILED for command %s\n", first_command);
	}
	else
	{
		wait(&status);
		//tcsetpgrp(0, getpgid(pid));
	}
}

void InteractiveMode()
{
	int historycount = 0;

	printf("Entering Interactive Mode...\n\n");
	while (1)
	{
		EXIT_CALLED = 0; //setting exit command to no
		//if history has been previously deleted, reset historycount
		//or if there is a function in history that is to be executed, execute it
		if (COMMANDS[0] != NULL)
		{
			if (strcmp(COMMANDS[0], "cleared") == 0)
			{
				historycount = 0;
				COMMANDS[0] = NULL;
			}
			else if (strcmp(COMMANDS[0], "historyexecute") == 0)
			{
				char *str = (char *)malloc(MAX);
				str = COMMANDS[1]; //give history -e command to str
				int cmd_num = ParseCommands(str);
				for (int i = 0; i < cmd_num; i++)
				{
					char *temp = strdup(COMMANDS[i]);
					temp = strtok(temp, " ");
					ExecuteCommands(temp, COMMANDS[i]);
				}
				free(str);
				COMMANDS[0] = NULL;
				COMMANDS[1] = NULL;
			}
		}

		char *str = (char *)malloc(MAX);

		printf("Enter a command: ");
		fgets(str, MAX, stdin);

		//check if char* is NULL. If only an enter or return key is pressed, str pointer holds str[0]=='\n' and str[1]=='\0'
		//so to see if a string is empty, check if str[0]=='\n'
		while ((str != NULL) && (str[0] == '\n'))
		{
			EXIT_CALLED = 0; //exit command bool is no
			printf("Enter a command: ");
			fgets(str, MAX, stdin);
		}

		//while there are aliases stored, iterate through all to see if an alias matches
		for (int i = 0; i < MAX; ++i)
		{
			if (MYALIAS[i] == NULL)
			{
				break;
			}

			//remove newline from str bc fgets stores newline
			char *temp = strdup(str);
			temp = strtok(temp, "\n");

			//compare to see if user command has an alias command
			if ((strcmp(temp, MYALIAS[i]) == 0))
			{
				//if there is a corresponding alias command, replace the current command with that
				str = strdup(MYALIASCOMMAND[i]);
			}
		}

		//if history array is full, reset count
		if (historycount == 20)
		{
			historycount = 0;
		}

		//keeps track of entire line for history
		char *historytoken = strtok(str, "\n"); //gets rid of endline so when history is called there is no space between histories
		MYHISTORY[historycount] = strdup(historytoken);
		historycount++;

		//parse commands
		int cmd_num = ParseCommands(str); //this function can be better designed

		//execute commands that are saved in COMMANDS[] array
		for (int i = 0; i < cmd_num; i++)
		{
			char *temp = strdup(COMMANDS[i]);
			temp = strtok(temp, " ");
			ExecuteCommands(temp, COMMANDS[i]);
		}
		//ctrl-d kill

		// if exit was selected
		if (EXIT_CALLED)
			MyExit();

		//frees str that holds user input
		free(str);
	}
}

void MyExit()
{
	//clean up
	setenv("PATH", ORIG_PATH_VAR, 1); //Hien's part - reset path to original value
	exit(0);
}

void MyPath(char *args[], int arg_count)
{
	if (arg_count != 1 && arg_count != 3)
	{
		printf("Invalid number of arguments for path command\n");
		return;
	}
	if (arg_count == 1)
	{
		pathRead();
	}
	else if (strcmp(args[1], "+") == 0)
	{
		pathAppend(MYPATH, args);
		strcpy(MYPATH, getenv("PATH"));
	}
	else if (strcmp(args[1], "-") == 0)
	{
		pathRemove(MYPATH, args);
		strcpy(MYPATH, getenv("PATH"));
	}
	else
	{
		printf("Invalid arguments for path command\n");
		return;
	}
}

//void PipeCommands(char *args[], char *first_command, int arg_count)
//Richard Gallemore 3600.002
//Function Desciprtion: Piping is a method in c programming which utilizes the flow of information between different pipes. Essentially, it passes information using the method of forking and is crucial in for multithreading. It will pass the output of one pipe into the input of another pipe between different processes
//The code was modified from a form that would use one pipe into a new method that would allow up to two pipes maximum, since there is a maximum of 3 arguments
void PipeCommands(char *full_line)
{
	//int filedes[2];//temp?
	pid_t pid;
	int status;
	pid_t wpid;
	int pipe1[2]; //first bar
	int pipe2[2]; //second bar
	int command_count;
	char *command[512];
	//printf("Before command count function\n");
	//write code to parse full command line by | here, get command_count and put each command into the command[] array. Hint: review the Professor's ParseCommands() function to do this part
	// For ex, if full line is who | wc -l, command={"who", "wc -l"}
	/*	for(int i = 0; i < command_count; i++){
		printf("\nTHE COMMANDS ARE %s\n", COMMANDS[i]);
	}*/
	command_count = ParseCommandsPipe(full_line);
	//rintf("In pipe commands function\n");
	printf("full line is %s\n", full_line);
	// if we have 2 commands in the line, or if there is only one pipe
	if (command_count == 2)
	{
		// create pipe1
		if (pipe(pipe1) == -1)
		{
			perror("bad pipe1"); //display if fail to create pipe
			exit(1);
		}

		// fork command 1
		if ((pid = fork()) < -1)
		{
			perror("bad fork"); //display if fail to create children
			exit(1);
		}
		else if (pid == 0)
		{
			close(pipe1[0]);				//close unused pipe
			dup2(pipe1[1], fileno(stdout)); //instead of print out in file descriptor stdout, put the output in pipe
			close(pipe1[1]);				//close when finish using

			//execute command 1
			exec(command[0]); //temp tl
			exit(0);
		}
		// parent
		else
		{
			do
			{
				//wait until chidlren finish and terminate
				wpid = waitpid(pid, &status, WUNTRACED);
			} while (!WIFEXITED(status) && !WIFSIGNALED(status));
			if (WIFEXITED(status))
			{
				pid = fork();
				if (pid < 0)
				{
					perror("bad fork"); //display if fail to fork
					exit(1);
				}
				else if (pid == 0)
				{
					close(pipe1[1]);			   //close unsused pipe
					dup2(pipe1[0], fileno(stdin)); //read the output of the first command into pipe1, instead of stdin
					close(pipe1[0]);			   //close when finish using

					//execute command 2
					exec(command[1]);
					exit(0); //exit after done
				}
				else
				{
					close(pipe1[0]); //close unused pipe
					close(pipe1[1]); //close unused pipe

					//parent wait until the children terminate
					do
					{
						wpid = waitpid(pid, &status, WUNTRACED);
					} while (!WIFEXITED(status) && !WIFSIGNALED(status));
				}
			}
		}
	} //this next section for two pipes
	else if (command_count == 3)
	{ //two pipes and 3 arguments
		printf("For 2 pipes\n");
		// create pipe1
		if (pipe(pipe1) == -1)
		{
			perror("bad pipe2"); //display if fail to create pipe
			exit(1);
		}

		// fork command 1
		if ((pid = fork()) < -1)
		{
			perror("bad fork"); //display if fail to create children
			exit(1);
		}
		else if (pid == 0)
		{
			close(pipe1[0]);				//close unused pipe
			dup2(pipe1[1], fileno(stdout)); //instead of print out in file descriptor stdout, put the output in pipe
			close(pipe1[1]);				//close when finish using

			//execute command 1
			exec(command[0]); //temp tl
			exit(0);
		}
		// parent
		else
		{
			do
			{
				//wait until chidlren finish and terminate
				wpid = waitpid(pid, &status, WUNTRACED);
			} while (!WIFEXITED(status) && !WIFSIGNALED(status));
			if (WIFEXITED(status))
			{
				pid = fork();
				if (pid < 0)
				{
					perror("bad fork"); //display if fail to fork
					exit(1);
				}
				else if (pid == 0)
				{
					close(pipe1[1]);			   //close unsused pipe
					dup2(pipe1[0], fileno(stdin)); //read the output of the first command into pipe1, instead of stdin
					close(pipe1[0]);			   //close when finish using

					//execute command
					exec(command[1]);
					dup2(pipe2[0], fileno(stdin)); //redirection //read the input of the first command into pipe2
					close(pipe2[0]);
					exec(command[2]); //similar to for one pipe but more layers for two pipes
					exit(0);		  //exit after done
				}
				else
				{
					close(pipe1[0]); //close unused pipe
					close(pipe1[1]); //close unused pipe

					//parent wait until the children terminate
					do
					{
						wpid = waitpid(pid, &status, WUNTRACED);
					} while (!WIFEXITED(status) && !WIFSIGNALED(status));
				}
			}
		}
	}
}
//parsing for pipe count
int ParseCommandsPipe(char *str)
{ //was str
	printf("\npipe parse string is %s\n", str);
	printf("Parse000\n");
	int i = 0;
	//char str1[] = "Geeks-for-Geeks";
	char *token = strtok(str, "|"); //breaks str into a series of tokens using |
	printf("Parse001\n");
	/*	while (token != NULL){
		printf("%s\n", token);
		token = strtok(NULL, "-");
	}*/
	while (token != NULL)
	{
		token = trimWhiteSpace(token);
		if (isSpace(token) != 0)
		{
			COMMANDS[i] = token;
			char *temp = COMMANDS[i];
			if (temp[0] != '\n')
			{
				i++;
			}
		}
		token = strtok(NULL, ";\n");
	}
	printf("Parse002\n");
	char *token2 = strtok(str, "\n");

	//if there is only 1 command with no semi colon at the end. E.g "ps u"
	while (token != NULL)
	{
		//printf(" %s\n", token);

		token = trimWhiteSpace(token);
		if (isSpace(token) != 0)
		{
			//token = trimWhiteSpace(token);
			COMMANDS[i] = token;

			char *temp = COMMANDS[i];
			if (temp[0] != '\n')
			{
				i++;
			}
		}
	}

	printf("Total # of commands: %d\n", i); //mytest
	return i;
}

void exec(char *command)
{
	char *args[50];
	//Parse the command by white space using ParseArgs()

	//call execvp
	execvp(args[0], args);
	//if failed print message
	perror("Failed");
	exit(0);
}

void signalHandle(int sig)
{
	switch (sig)
	{
	case SIGINT:
		signal(SIGINT, SIG_DFL);
		break;
	case SIGTSTP:
		signal(SIGTSTP, SIG_DFL);
		break;
	}
}
