/*
* Project 2: Part 2 CIS 415 University of Oregon
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

pid_t *array;
pid_t *arraytmp;
int processes;
int terminated = 0;
FILE *stream;
char *line = NULL;
extern int errno;

// Sends signal to each child in array
void signaler(pid_t* array, int signal) {
  for(int i = 0; i < 5; i++) {
    sleep(1);
    printf("=== MCP: <%d> - Sending signal: <%d> to process: <%d>\n", getpid(), signal, array[i]);
    int catch = kill(array[i], signal);
    if (catch == -1) {
      printf("%s\n", strerror(errno));
    }
  }
}

// Signal handler for SIGUSR1
void signalHandler(int sig) {
//printf("Process: %d - Received signal: <%d> - Calling exec().\n", getpid(), sig);
}

pid_t callExec(char** arguments) {
  pid_t pid;
  int sig;
  int curP;
  // Create SIGUSR1 sigaction/handler
  sigset_t signals;
  sigemptyset( &signals );
  sigaddset( &signals , SIGUSR1);
  struct sigaction action = { 0 }; // { 0 } removes valgrind warning of uninitialized bytes
  action.sa_handler = signalHandler;
  sigaction(SIGUSR1, &action, NULL);

  fflush(stdout);
  // Fork process
  pid = fork ();
  // In child wait for SIGUSR1 then call exec on arguments[0] with arguments
  if (pid == 0)
  {
    printf("   === Process: %d - Waiting for SIGUSR1...\n", getpid());
    sigwait(&signals, &sig);
    printf("   === Process: %d - Received signal: <%d> - Calling execvp(%s).\n", getpid(), sig, arguments[0]);
    fflush(stdout);
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
    // Do nothing if parent
  }
}

int main(int argc, char* argv[]) {
  // If not one input file, exit and print why
  if(argc != 2) {
    printf("Error: Must enter <1> input file.\n");
    exit(-1);
  }
  // Vars for input and parsing input
  line = NULL;
  size_t len = 0;
  ssize_t nread;
  stream = fopen(argv[1], "r");
  char *word;
  char *token;
  // count of processes, and array of children hard coded at a number.
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
  sleep(1);
  // Send SIGUSR1 to all
  signaler(children, SIGUSR1);
  // Send SIGSTOP to all
  signaler(children, SIGSTOP);
  // Send SIGCONT to all
  signaler(children, SIGCONT);
  // Wait for all
  for(int i = terminated; i < processes; i++) {
    waitpid(children[i], 0, 0);
  }
  // Exit MCP
  free(line);
  fclose(stream);
  exit(0);
}
