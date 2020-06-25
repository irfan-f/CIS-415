/*
* Project 2: Part 4 CIS 415 University of Oregon
* Author: Irfan Filipovic
* Date: 05/10/20
* NOTES:
* 1. Has an error of jump or move on uninitialized variable, however it does not halt the program. Merely warning.
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
#include <fcntl.h>

pid_t currentProcess = 0;
pid_t arraytmp[60];
int processes;
int terminated = 0;
FILE *stream;
char *line = NULL;
extern int errno;


int maxWait = 5; // Seconds for alarm to sound on

// PART 4 code
// We want Name[0], State[2], Pid[5], VMSize[17], VMData[25], VMstk[26], voluntary switches[53], Nonvoluntary switches[54]
// All VMs are address space used at state, so current is Size, stk is stack size
// data is total without stack and shared.
// switches will see how we managed the processes
void part4(pid_t *p) {
  system("clear");
  char *data = (char*) malloc(2048*sizeof(char));
  char *data2 = (char*) malloc(2048*sizeof(char));
  ssize_t bytes = 0;
  ssize_t bytes2 = 0;
  int fd;
  int fd2;
  char *line;
  for (int i = 0; i < processes; i++) {
    char file[60];
    char file2[60];

    char *pidData[60];
    char *dataShow[11] = {0};

    if(p[i] == 0) {
      continue;
    }
    // sprintf, is printf but prints to buffer, allows for formatting strings easily
    sprintf(file, "/proc/%d/status", p[i]);
    fd = open(file, O_RDONLY);
    if ( fd == -1) {
      printf("Error opening '/proc/' file.\n");
    }
    bytes = read(fd, data, 2048);
    if(bytes == -1) {
      perror("read");
      free(data);
      close(fd);
    }
    line = strtok(data, "\n");
    pidData[0] = line;
    int z = 1;
    while((line = strtok(NULL, "\n")) != NULL) {
      pidData[z] = line;
      z++;
    }
    // Name, State, Pid, VMSize, VMData, VMStk, Voluntary, Non-Voluntary
    dataShow[0] = pidData[0];
    dataShow[1] = pidData[2];
    dataShow[2] = pidData[5];
    dataShow[3] = pidData[17];
    dataShow[4] = pidData[25];
    dataShow[5] = pidData[26];
    dataShow[6] = pidData[53];
    dataShow[7] = pidData[54];

    // Get io data
    // USING ONLY read chars and written chars
    sprintf(file2, "/proc/%d/io", p[i]);
    fd2 = open(file2, O_RDONLY);
    if ( fd2 == -1) {
      printf("Error opening '/proc/' file.\n");
    }
    bytes2 = read(fd2, data2, 2048);
    if(bytes2 == -1) {
      perror("read");
      free(data2);
      close(fd2);
    }
    close(fd2);
    line = strtok(data2, "\n"); // Read chars
    dataShow[8] = line;
    line = strtok(NULL, "\n"); // Written Chars
    dataShow[9] = line;

    // All data shown here is what i found important or interesting
    printf("========== Process %d ==========\n", p[i]);
    for(int j = 0; j < 10; j++) {
      printf("%s\n", dataShow[j]);
    }
  }
  free(data);
  free(data2);
  close(fd);
  close(fd2);
}


// Sends signal to each child in array
void signaler(pid_t* array, int signal) {
  for(int i = 0; i < processes; i++) {
    sleep(1);
    //printf("=== MCP: <%d> - Sending signal: <%d> to process: <%d>\n", getpid(), signal, array[i]);
    int catch = kill(array[i], signal);
    if (catch == -1) {
      //printf("%s\n", strerror(errno));
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
    //printf("   === Process: <%d> - Waiting for SIGUSR1...\n", getpid());
    sigwait(&signals, &sig);
    //printf("   === Process: <%d> - Received signal: <%d> - Calling execvp(%s).\n", getpid(), sig, arguments[0]);
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
    //printf("   === Process: %d -> Finished\n", arraytmp[index]);
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


void alarmHandler(int s) {
  //printf("####### Alarm Sounded: Switching Processes #######\n");
  //printf("####### Processes remaining: %d.\n", processes);
  if (processes == 1) {
    //printf("####### Finishing last process #######\n");
    //printf("   === Process %d: Being continued.\n", arraytmp[0]);
    kill(arraytmp[0], SIGCONT);
  } else {
    for(int i = 0; i < processes; i++) {
      if(currentProcess == i) {
        //printf("   === Process %d: Being stopped.\n", arraytmp[i]);
        kill(arraytmp[i], SIGSTOP);
        //printf("   === Process %d: Being continued.\n", arraytmp[(i+1)%processes]);
        kill(arraytmp[(i+1)%processes], SIGCONT);
        currentProcess = (i+1)%processes;
        break;
      }
    }
  part4(arraytmp);
  alarm(maxWait);
  signal(SIGALRM, alarmHandler);
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
  pid_t children[60];

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
  for(int i = 0; i < processes; i++) {
    arraytmp[i] = children[i];
  }
  sleep(1);
  // Send SIGUSR1 to all
  signaler(children, SIGUSR1);
  // Send SIGSTOP to all
  signaler(children, SIGSTOP);
  // action handler for SIGCHLD
  struct sigaction action2 = { 0 }; // { 0 } removes valgrind warning of uninitialized bytes
  action2.sa_handler = signalHandler2;
  sigaction(SIGCHLD, &action2, NULL);
  // Create alarm and call handler on maxWait seconds
  currentProcess = 0;
  signal(SIGALRM, alarmHandler);
  alarm(maxWait);
  // Wait for all
  while(processes != 0) {
    ;
  }
  // Exit === MCP
  free(line);
  fclose(stream);
  exit(0);
}
