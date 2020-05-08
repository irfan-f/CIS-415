/*
* Lab 4 CIS 415 University of Oregon
* Author: Irfan Filipovic
* Date: 04/24/20
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

pid_t array[5];

void signaler(pid_t* array, int signal) {
  for(int i = 0; i < 5; i++) {
    sleep(1);
    printf("Parent process: <%d> - Sending signal: <%d> to child process: <%d>\n", getpid(), signal, array[i]);
    int catch = kill(array[i], signal);
    if (catch == -1) {
      printf("%s\n", strerror(errno));
    }
  }
}

void signalHandler(int sig) {
  printf("Child Process: <%d> - Received signal: <%d> - Calling exec().\n", getpid(), sig);
}

void signal2Handler(int sig) {

}
int main() {
  // Array for multiple proccesses, fork each
  pid_t pid;
  int sig;
  sigset_t signals;
  sigemptyset( &signals );
  sigaddset( &signals , SIGUSR1);
  struct sigaction action;
  action.sa_handler = signalHandler;
  sigaction(SIGUSR1, &action, NULL);

  for(int i = 0; i < 5; i++)
  {
    fflush(stdout);
    pid = fork ();
    if (pid == 0)
    {
      printf("Child Process: <%d> - Waiting for SIGUSR1...\n", getpid());
      sigwait(&signals, &sig);
      printf("Child Process: <%d> - Received signal: <%d> - Calling exec().\n", getpid(), sig);
      fflush(stdout);
      // Execute on binary
      if (execv("./iobound", NULL) < 0)
      {
        perror("execv");
      }
    }
    else
    {
      array[i] = pid;
    }
  }
  signaler(array, SIGUSR1);
  signaler(array, SIGINT);
}
