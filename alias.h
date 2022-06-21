#ifndef ALIAS_H
#define ALIAS_H

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

void Alias(char *args[], char *full_line, char *MYALIAS[], char *MYALIASCOMMAND[]);

#endif