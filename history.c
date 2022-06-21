//Name: Khang Nguyen
//Course: CSCE 3600.002
//Date:4-18-2021
//Description: This program adds a built in myhistory command. This command displays the 20 most recent commands typed into the shell.
//The user has the ability to clear the history or to execute a command from the history.
#include "history.h"

void MyHistory(char *args[], int arg_count, char *MYHISTORY[],char*COMMANDS[])
{
    //if history has an argument
    if (args[1] != NULL)
    {
        char *temp = args[1]; //holds history argument, either -c or -e
        char *temp2 = args[2]; //holds argument of -e (the argument is the line number)
        int val;               //value to hold -e argument history line number

        //check if argument is to clear history
        if (strcmp(temp, "-c") == 0)
        {
            //clear history
            for (int i = 0; i < 20; ++i)
            {
                MYHISTORY[i] = NULL;
            }
            printf("History Successfully Cleared\n");
            COMMANDS[0] = "cleared"; //set message that is checked when interactive mode runs again
            return;
        }

        //check if argument is to execute a command from history
        if (strcmp(temp, "-e") == 0)
        {
            //turn line num argument to integer
            if (temp2 != NULL)
            {
                val = atoi(temp2);
            }
            COMMANDS[0] = "historyexecute";           //set message that is checked when interactive mode runs again
            COMMANDS[1] = strdup(MYHISTORY[val - 1]); //get command and give it to command[1]
            return;
        }
    }

    //print the history
    printf("History:\n");

    for (int i = 0; i < 20; ++i)
    {
        //print only the entries that have something
        if (MYHISTORY[i] != NULL)
        {
            printf("[%d]: %s\n", i + 1, MYHISTORY[i]);
        }
    }
}