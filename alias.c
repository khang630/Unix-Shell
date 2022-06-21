//Name: Khang Nguyen
//Course: CSCE 3600.002
//Date:4-18-2021
//Description: This program adds the alias shell support function. This function allows a user to give a command a certain alias.
//The user has the ability to make aliases, delete a specific alias, or delete all aliases.
#include "alias.h"

void Alias(char *args[], char *full_line, char *MYALIAS[], char *MYALIASCOMMAND[])
{
    int arrcount = 0;

    //if only alias typed with no arguments, display a list of all existing aliases
    if (args[1] == NULL)
    {
        int aliascount = 0;
        if (MYALIAS[0] == NULL) //if alias array is empty
        {
            printf("No aliases to display: \n");
            return;
        }
        printf("List of existing aliases: \n"); //list all aliases
        for (int i = 0; i < 512; ++i)
        {
            //exit if end of array reached
            if (MYALIAS[i] == NULL)
            {
                return;
            }

            //if index has placeholder value, don't display it. Instead, skip over it and continue
            if ((strcmp(MYALIAS[i], "deleted") != 0))
            {
                //if alias has no command with it
                if (MYALIASCOMMAND[i] == NULL)
                {
                    continue;
                }
                printf("[%d] %s = %s\n", aliascount + 1, MYALIAS[i], MYALIASCOMMAND[i]);
                aliascount++;
            }
        }
    }

    //printf("Alias called\n"); //test
    if (args[1] != NULL)
    {
        //remove a specific alias
        if (strcmp(args[1], "-r") == 0)
        {
            for (int i = 0; i < 512; ++i)
            {
                //leave if end of array
                if (MYALIAS[i] == NULL)
                {
                    printf("Failed to delete alias, alias '%s' does not exist:\n", args[2]); //fix this
                    return;
                }

                //if alias matches user input, "delete"
                if (strcmp(MYALIAS[i], args[2]) == 0)
                {

                    //check if index is last in array, if so, just set to NULL
                    if (MYALIAS[i + 1] == NULL)
                    {
                        MYALIAS[i] = NULL;
                        MYALIASCOMMAND[i] = NULL;
                        return;
                    }
                    else
                    {
                        //if there are other values after this index, give it a garbage value so loop doesn't exit prematurely
                        //loop iterates and exits if there is a null value encountered, so to circumvent this we give this index something so that it isn't null
                        MYALIAS[i] = "deleted";
                        MYALIASCOMMAND[i] = "deleted";
                        return;
                    }
                }
            }
            return;
        }

        //remove all aliases
        if (strcmp(args[1], "-c") == 0)
        {
            printf("All aliases cleared:\n");
            //clear array
            for (int i = 0; i < 512; ++i)
            {
                //leave if end of array
                if (MYALIAS[i] == NULL)
                {
                    return;
                }

                MYALIAS[i] = NULL;
                MYALIASCOMMAND[i] = NULL;
            }
            return;
        }

        char *aliasname = strtok(args[1], "=");

        //increment to an open spot in both arrays
        while (MYALIAS[arrcount] != NULL)
        {
            arrcount++;
        }

        //store aliasname
        MYALIAS[arrcount] = aliasname;

        //skips first token before ' and takes the second token before 2nd '
        //takes the command in between the '' in (alias name='command')
        char *aliascommand = strtok(full_line, "'");
        aliascommand = strtok(NULL, "'");

        //If command is NULL, this means the format for alias command was wrong.
        if (aliascommand == NULL)
        {
            return;
        }

        //checks to see if the format is (alias name='command'). There should be something inside the ''.
        //if correct store the command, if the stored command is NULL leave
        MYALIASCOMMAND[arrcount] = strdup(aliascommand);
        if (MYALIASCOMMAND[arrcount] == NULL)
        {
            return;
        }
    }
}