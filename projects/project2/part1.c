/*
* Project 2: Part 1 CIS 415 University of Oregon
* Author: Irfan Filipovic
* Date: 05/10/20
* NOTES:
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

FILE *stream;
char *line = NULL;
int processes;
int terminated = 0;
extern int errno;

pid_t callExec(char** arguments) {
  // Array for multiple proccesses, fork each
  pid_t pid;
  pid = fork ();
  if (pid == 0)
  {
    printf("Process: %d -Calling execvp %s.\n", getpid(), arguments[0]);
    // Execute on binary
    if (execvp(arguments[0], arguments) < 0)
    {
      free(line);
      fclose(stream);
      perror("execvp");
    }
    exit(0);
  }
  else
  {
    ;
  }
}

int main(int argc, char* argv[]) {
  line = NULL;
  size_t len = 0;
  ssize_t nread;
  stream = fopen(argv[1], "r");
  char *word;
  char *token;

  int count = 0;
  pid_t children[6];

  while(getline(&line, &len, stream) != -1) {
    int subcount = 0;
    /* Tokenize the input string */
    word = line;
    char *tmparg[strlen(word)/4];
    token = strtok_r(word, " ", &word);
    while (token != NULL) {
      size_t tokenLen = strcspn(token, "\n");
      token[tokenLen] = '\0';
      tmparg[subcount] = token;
      token = strtok_r(NULL, " ", &word);
      subcount++;
    }
    tmparg[subcount] = (char *) NULL;
    for(int i = 0; i < subcount; i++) {
      ;
    }
    children[count] = callExec(tmparg);
    count++;
  }
  processes = count;
  for(int i = terminated; i < processes; i++) {
    waitpid(children[i], 0, 0);
  }
  free(line);
  fclose(stream);
  exit(0);
}
