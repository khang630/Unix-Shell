//Name: Hien Pham
//Course: CSCE 3600.002
//Date:
//Description: This program adds functions to the main major2.c program. The functions allows the user to manipulate and read the shell's path.
#include "path.h"

void pathAppend(char *myPath, char *args[])
{
	//string to hold pathname to be added
	char *pathAdded;
	pathAdded = (char *)malloc(1024);

	//malloc error checking
	if(pathAdded == NULL)
	{
		printf("Error allocating memory during path command\n");
		return;
	}
	memset(pathAdded, '\0', sizeof(pathAdded));

	//set the put the pathname to be added into the string
	strncpy(pathAdded, args[2], 1024);

	//check for special cases
	if(pathAdded[0] == '$')
	{
		//remove the first character and get the directory
		memmove(pathAdded, pathAdded + 1, strlen(pathAdded));
		//error checking
		if(getenv(pathAdded) == NULL)
		{
			printf("$%s is not found\n", pathAdded);
			free(pathAdded);
			return;
		}
		strncpy(pathAdded, getenv(pathAdded), 1024);
	}
	else if(pathAdded[0] == '~')
	{
		//string to hold directories after the home directory
		char *temp;
		temp = (char *)malloc(1024);
		//malloc error checking
		if(temp == NULL)
		{
			printf("Error allocating memory during path command\n");
			free(pathAdded);
			return;
		}
		//remove ~
		memset(temp, '\0', sizeof(temp));
		memmove(pathAdded, pathAdded + 1, strlen(pathAdded));

		//get the home directory and add it to temp
		strncpy(temp, getenv("HOME"), 1024);
		//get the other directories and add it to temp
		strncat(temp, pathAdded, 1024 - strlen(temp));

		//copies temp to pathadded
		strncpy(pathAdded, temp, 1024);

		//free memory
		free(temp);
	}

	//adds path to be added to the path
	strncat(myPath, ":", 1024 - (int)strlen(myPath));
	strncat(myPath, pathAdded, 1024 - (int)strlen(myPath));

	//change the path
	if(setenv("PATH", myPath, 1) == -1)
	{
		printf("Error changing path\n");
		free(pathAdded);
	}

	//free memmory
	free(pathAdded);
}
void pathRemove(char *myPath, char *args[])
{
	//string to hold the pathname that will replace the current pathname
	char *newPath;
	newPath = (char *)malloc(1024);

	//malloc error checking
	if(newPath == NULL)
	{
		printf("Error allocating memory during path command\n");
		return;
	}
	memset(newPath, '\0', sizeof(newPath));

	//string to hold the pathname to be removed
	char* pathRemoved;
	pathRemoved = (char *)malloc(1024);

	//malloc error checking
	if(pathRemoved == NULL)
	{
		printf("Error allocating memory during path command\n");
		free(newPath);
		return;
	}
	memset(pathRemoved, '\0', sizeof(pathRemoved));

	//put the pathname ot be added into the string
	strncpy(pathRemoved, args[2], 1024);

	//check for special cases
	if(pathRemoved[0] == '$')
	{
		//remove the first character to get the directory
		memmove(pathRemoved, pathRemoved + 1, strlen(pathRemoved));
		//error checking
		if(getenv(pathRemoved) == NULL)
		{
			printf("$%s is not found\n", pathRemoved);
			free(newPath);
			free(pathRemoved);
			return;
		}
		strncpy(pathRemoved, getenv(pathRemoved), 1024);
	}
	else if(pathRemoved[0] == '~')
	{
		//string to hold the directories after the home directory
		char *temp;
		temp = (char *)malloc(1024);
		//malloc error checking
		if(temp == NULL)
		{
			printf("Error allocating memory during path command\n");
			free(newPath);
			free(pathRemoved);
			return;
		}
		//remove ~
		memset(temp, '\0', sizeof(temp));
		memmove(pathRemoved, pathRemoved + 1, strlen(pathRemoved));

		////get the home directory and add it to temp
		strncpy(temp, getenv("HOME"), 1024);
		//get the other directories and add it to temp
		strncat(temp, pathRemoved, 1024 - strlen(temp));

		//copies temp to pathremoved
		strncpy(pathRemoved, temp, 1024);

		//free memory
		free(temp);
	}

	//split up the path into token separated by :
	char *token = strtok(myPath, ":");
	//bool value to check to see if the path to be removed has been found yet
	int found = 1;

	//Adds each pathname that is not the pathname to be removed into the new path
	if(strcmp(token, pathRemoved) != 0)
	{
		strncat(newPath, token, 1024 - (int)strlen(myPath));
	}
	else
	{
		//if found set found to true. does not add the first path it found into the new path
		if(found == 1)
		{
			found = 0;
		}
		else
		{
			strncat(newPath, token, 1024 - (int)strlen(newPath));
		}
	}
	//next pathname
	token = strtok(NULL, ":");

	//same thing as above but as a : before. This is to make sure the path does not end with a colon
	while(token != NULL)
	{
		if(strcmp(token, pathRemoved) != 0)
        	{
			strncat(newPath, ":", 1024 - (int)strlen(newPath));
                	strncat(newPath, token, 1024 - (int)strlen(myPath));
        	}
       		else
        	{
                	if(found == 1)
                	{
                	        found = 0;
                	}
                	else
                	{
				strncat(newPath, ":", 1024 - (int)strlen(newPath));
                	        strncat(newPath, token, 1024 - (int)strlen(myPath));
                	}
        	}	
        	token = strtok(NULL, ":");
	}
	//set the new path
	strncpy(myPath, newPath, 1024);
	//error checking
	if(setenv("PATH", myPath, 1) == -1)
	{
		printf("Error changing path\n");
		free(newPath);
		free(pathRemoved);
		return;
	}

	//free memoty
	free(newPath);
	free(pathRemoved);
}
void pathRead()
{
	char *output;
	output = getenv("PATH");
	printf("%s\n", output);
}
