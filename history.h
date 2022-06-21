#ifndef HISTORY_H
#define HISTORY_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

void MyHistory(char *args[], int arg_count, char *MYHISTORY[],char*COMMANDS[]);

#endif