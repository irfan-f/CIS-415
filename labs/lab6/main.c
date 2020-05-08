/*
* Lab 6 CIS 415 University of Oregon
* Author: Irfan Filipovic
* Date: 05/05/20
* NOTES:
* 1. I lowered 'iobound.c' var number by 2 magnitudes as to speed up testing
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
pid_t arraytmp[5];
int processes = 5;

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

// Calls SIGSTOP and SIGCONT on processes in array, while num processes when called is equal to current num processes
void signaler2(pid_t *array, int numP) {
  // Till one child exits
  if (numP == 1) {
    // if one process just continue it and end
    printf("Finishing the last process: %d\n", array[0]);
    kill(array[0], SIGCONT);
  }
  while(numP == processes) {
    // If more than one process
    if(numP != 1) {
      // go through proccesses
      for(int i = 0; i < numP; i++) {
        // send SIGSTOP
        kill(array[i], SIGSTOP);
        if(processes > 1) {
          printf("Stopping this process: %d\n",array[i]);
        }
        // send SIGCONT to next,  //if i == 4 and i + 1 == 5 % 5 = 0   // implement circular array
        kill(array[(i+1)%5], SIGCONT);
        if(processes > 1) {
          printf("Continuing this process: %d\n", array[(i+5)%5]);
        }
        // sleep for 1 second
        sleep(1);
      }
    }
  }
}

void signalHandler(int sig) {
  printf("Child Process: <%d> - Received signal: <%d> - Calling exec().\n", getpid(), sig);
}

// Handles signal SIGCHLD sent when child terminates
void signalHandler2(int sig) {
  // variables for pid that exits, status it exits with, and index of elem that exited
  pid_t pid;
  int status;
  int index;
  // waitpid will return the pid of any (-1) process that terminates
  // WNOHANG makes it return if no terminate so asynchronous not blocking
  while((pid = waitpid(-1, &status, WNOHANG)) != -1) {
    // 0 indicates no change, break while
    if(pid == 0){ break ;}
    // loop through arrays and see which index matches pid that terminated
    for(int i = 0; i < processes; i++) {
      if(arraytmp[i] == pid) {
        index = i;
      }
    }
    printf("Process: %d -> Finished\n", arraytmp[index]);
    // lower num of processes as one terminated
    processes--;
    // if index is last elem it will be ignored
    if(index != processes) {
      // else then we must shift the array over the process that ended
      // dont alter array as it messes up running children processes so tmp must be used
      for(int j = index; j < processes; j++) {
        arraytmp[j] = arraytmp[j+1];
      }
    }
  }
}
int main() {
  // Array for multiple proccesses, fork each
  pid_t pid;
  int sig;
  int curP;
  sigset_t signals;
  sigemptyset( &signals );
  sigaddset( &signals , SIGUSR1);
  struct sigaction action = { 0 }; // { 0 } removes valgrind warning of uninitialized bytes
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
      exit(0);
    }
    else
    {
      array[i] = pid;
    }
  }
  for(int i = 0; i < processes; i++) {
    arraytmp[i] = array[i];
  }
  // action handler for SIGCHLD
  struct sigaction action2 = { 0 }; // { 0 } removes valgrind warning of uninitialized bytes
  action2.sa_handler = signalHandler2;
  sigaction(SIGCHLD, &action2, NULL);

  signaler(array, SIGUSR1);
  // While there are children
  while(processes != 0) {
    // Get current number children, call signaler2
    curP = processes;
    signaler2(arraytmp, curP);
  }
  // removed this line as it sends to children that already terminate
  //signaler(array, SIGINT);
}
