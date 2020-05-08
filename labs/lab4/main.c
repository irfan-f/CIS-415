/*
* Lab 4 CIS 415 University of Oregon
* Author: Irfan Filipovic
* Date: 04/24/20
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
  // Array for multiple proccesses, fork each
  pid_t pid;

  printf("mian start here my pid is %d\n", getpid());

  pid = fork ();

  if (pid == 0)
  {
    printf("This is the child process, my pid is %d, my parent pid is %d\n", getpid (), getppid());
    printf("My status is %d\n\n", pid);
    // Execute on binary
    /*
    if (execv("./helloWorld", NULL) < 0)
    {
      perror("execv");
    }
    */
    sleep(2);
  }
  else
  {
    wait (0);
    printf("Main existing, my pid is %d\n", getpid());
  }
}
