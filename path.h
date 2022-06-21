#ifndef PATH_H
#define PATH_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>


void pathAppend(char *myPath, char *args[]);
void pathRemove(char *myPath, char *args[]);
void pathRead();

#endif
