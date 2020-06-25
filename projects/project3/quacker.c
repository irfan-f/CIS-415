/*
* CIS 415: Operating Systems
* Professor Malony
* 06/08/2020
* Project 3
*
* Author: Irfan Filipovic
* NOTES:
*   Assistance from: Nedim Filipovic (my brother, does not go to school here), helped a lot with looping while not busy
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sched.h>
#include <signal.h>

#define MAXENTRIES 20
#define MAXTOPICS 5
#define URLSIZE 1024
#define CAPSIZE 100
#define MAXTNAME 30
#define NUMPROXIES 20
#define MAXTFILE 100
// To allow setting with delta command
double DELTA = 0.0;
// topicEntry struct from part 1
struct topicEntry;
typedef struct {
    int entryNum;
    struct timeval timeStamp;
    int pubID;
    char photoURL[URLSIZE];
    char photoCaption[CAPSIZE];
} topicEntry;
// topicQ struct from part 1
struct topicQ;
typedef struct {
    char name[MAXTNAME];
    int entries;
    topicEntry * buffer;
    int head;
    int tail;
    int length;
    int ID;
} topicQ;
// From sys/time.h
struct timeval;
// struct to pass file, threadID, and topicID, notRunning used to determine if thread should execute
struct publish;
typedef struct {
    int threadId;
    int topicId;
    char file[MAXTFILE];
    int notRunning;
} publish;
// struct to pass, notRunning used to determine if thread should execute
struct subscribe;
typedef struct {
    int threadId;
    char file[MAXTFILE];
    int notRunning;
} subscribe;
// struct to pass threadID, the queues to clean, and notRunning if cleaner thread should execute
struct cleanIt;
typedef struct {
    int threadId;
    int queues;
    int notRunning;
} cleanIt;
// Create a registry for all topics, each topic can have MAXENTRIES make buffers with topicEntry type for all
topicQ registry[MAXTOPICS];
topicEntry buffers[MAXTOPICS][MAXENTRIES];
// Publisher and subscriber threads, totla number threads should be NUMPROXIES + 1, cleaner made in main.
// Create argument arrays, only 1 cleaner
pthread_t pubs[NUMPROXIES/2];
publish pubargs[NUMPROXIES/2];
pthread_t subs[NUMPROXIES/2];
subscribe subargs[NUMPROXIES/2];
cleanIt cleaner_args;
// Mutex for each topic queue
pthread_mutex_t mutex[MAXTOPICS];
// Mutex and condition for thread_lock on updating notRunning and file
pthread_mutex_t thread_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int enqueue(topicEntry *entry, int queue) {
  // Queue full, return 0 as defined in project pdf, used to signify a yield
  if(registry[queue].buffer[registry[queue].head + 1].entryNum == -1) {
      return 0;
  } else {
    // Increase num entires, set entryNum of entry to entries val
    registry[queue].entries++;
    int num = registry[queue].entries;
    entry->entryNum = num;
    // get cur time and set to timestamp
    struct timeval cur;
    gettimeofday(&cur,NULL);
    entry->timeStamp = cur;
    // if not first enqueue on queue, change head val to match new entry pos
    // if head at buffer length, set to 0, else increment
    if(!((registry[queue].tail == registry[queue].head) && (registry[queue].buffer[registry[queue].head].entryNum == 0))){
        if(registry[queue].head == (registry[queue].length)) {
            registry[queue].head = 0;
        } else { registry[queue].head++; };
    }
    registry[queue].buffer[registry[queue].head] = *entry;
    return 1;
  }
}
// makes return entry with vals from registry
void copyEntry(topicEntry *entry, topicEntry* empty) {
  empty->entryNum = entry->entryNum;
  empty->timeStamp = entry->timeStamp;
  empty->pubID = entry->pubID;
  strcpy(empty->photoURL,entry->photoURL);
  strcpy(empty->photoCaption,entry->photoCaption);
}

int getEntry(int lastEntry, topicEntry *empty, int queue) {
  // Queue empty return 0 as defined in pdf
  if((registry[queue].head == registry[queue].tail) && (registry[queue].buffer[registry[queue].head].entryNum == 0)) { return 0; };
  int ind = registry[queue].tail;
  // while there are entries
  while (registry[queue].buffer[ind].entryNum != -1) {
    // If lastentry + 1 exists, return vals
    if(registry[queue].buffer[ind].entryNum == (lastEntry + 1)) {
      copyEntry(&registry[queue].buffer[ind], empty);
      // return successfull, last increments by 1
      return 1;
      // If entry but val is greater than lastEntry + 1, it was dequeued and get higher val
    } else if(registry[queue].buffer[ind].entryNum > (lastEntry + 1)) {
      copyEntry(&registry[queue].buffer[ind], empty);
      // return entryNum of lastEntry
      return registry[queue].buffer[ind].entryNum;
    }
    // If  not at end then increment, else restart circle
    if(ind != registry[queue].length) {
      ind++;
    } else {
      ind = 0;
    }
  }
  // case 3.i from pdf
  return 0;
}

void dequeue(int queue) {
  // If queue empty then do nothing
  if((registry[queue].head == registry[queue].tail) && (registry[queue].buffer[registry[queue].head].entryNum == 0)) {
    // nothing
  } else {
    // get current time value
    struct timeval now;
    gettimeofday(&now,NULL);

    double result = difftime(now.tv_sec, (registry[queue].buffer[registry[queue].tail].timeStamp.tv_sec));

    topicEntry null_entry = { .entryNum = -1 };
    topicEntry entry = { .entryNum = 0 };
    // While difference greater than DELTA, any entry after the oldest old entry won't be too old
    // If statements to account for circular buffer
    while(result >= DELTA) {
      // set removed, set entry to null entry
      registry[queue].buffer[registry[queue].tail] = null_entry;
      if(registry[queue].tail == registry[queue].length){
        registry[queue].buffer[registry[queue].tail - 1] = entry;
        if(registry[queue].tail == registry[queue].head){ registry[queue].head = 0; };
        registry[queue].tail = 0;
      // tail is 0, and entry at length is Empty
      // increment tail, if tail==head increment both.
      } else if(registry[queue].tail == 0) {
        registry[queue].buffer[registry[queue].length] = entry;
        if(registry[queue].tail == registry[queue].head){ registry[queue].head++; };
        registry[queue].tail++;
      // Null is at tail - 1, normal increment case
      } else {
        registry[queue].buffer[registry[queue].tail - 1] = entry;
        if(registry[queue].tail == registry[queue].head){ registry[queue].head++; };
        registry[queue].tail++;
      }
      // check if next entry not dequeued and exists
      if(registry[queue].buffer[registry[queue].tail].entryNum != -1) {
        gettimeofday(&now,NULL);
        result = difftime(now.tv_sec, (registry[queue].buffer[registry[queue].tail].timeStamp.tv_sec));
      } else { break; };
    }
  }
}

void *publisher(void *args) {
  int id = ((publish *)args)->threadId-1;
  while(1) {
  // Lock the threads
    pthread_mutex_lock(&thread_lock);
    // While thread is removed, wait
    while(((publish *)args)->notRunning) { pthread_cond_wait(&cond, &thread_lock); };
    // Unlock thread
    pthread_mutex_unlock(&thread_lock);
    // EXIT THREAD IF FILE PASSED IS CALLED "z", to allow freeing of threads
    sleep(2);
    if(!strcmp(((publish*)args)->file, "z")) { pthread_exit(NULL); };
    // Thread starting
    printf("Proxy thread %d- type: Publisher\n",((publish *)args)->threadId);
    sleep(2);
    FILE *fileC;
    fileC = fopen(((publish *)args)->file, "r");
    // if open failed exit
    if(fileC == NULL) { pthread_exit(NULL); };
    // Line parsing below with getline and comparisons with strcmp, Assistance from brother on parsing files effectively
    char *line = NULL;
    size_t characters;
    size_t bsize;
    // get line
    characters = getline(&line, &bsize, fileC);
    while(characters != -1) {
      char commands[1][10] = {0};
      int count = 0;
      // rather than strtok use this to use sscanf later on, merely copies each char until whitespace for command word
      for (int i = 0; i < characters;i++) {
        // get rid of newline too in case of stop\n being passed rather than stop
        if(line[i]!= ' ') {
          if(line[i] =='\n') {
            break;
          }
          commands[0][count] = line[i];
          count++;
        } else { break; };
      }
      if(!strcmp(commands[0],"put")) {
        // vars to scan into
        char tmp[4];
        int id;
        char url[URLSIZE];
        char caption[CAPSIZE];
        // Scan entry into tmp, &id, url, caption. id is only one that isn't a char
        // tmp is put, not needed
        // scans until url reaches end quote, and caption goes till new line to save spaces.
        int r = sscanf(line,"%s %d \"%[^\"]\" \"%[^\n\"]\"",tmp,&id,url,caption);
        // make entry with id, url, and caption
        topicEntry entry;
        entry.pubID = ((publish *)args)->threadId;
        strcpy(entry.photoURL,url);
        strcpy(entry.photoCaption,caption);
        // get index
        int topic_ID = -1;
        for (int queue = 0; queue < MAXTOPICS ; queue++) {
          if(registry[queue].ID == id) { topic_ID = queue; break; };
        }
        // topic does not exist
        if(topic_ID == -1) { pthread_exit(NULL); };
        // get mutex, loop till complete
        while(1) {
          pthread_mutex_lock(&(mutex[topic_ID]));
          int ind = enqueue(&entry,topic_ID);
          if(!ind) {
            // Wait for enqueue to process
            pthread_mutex_unlock(&(mutex[topic_ID]));
            sched_yield();
          } else {
            // Completed command, unlock mutex, end loop
            printf("Proxy thread %d- type: Publisher - Executed command: put\n",((publish *)args)->threadId);
            pthread_mutex_unlock(&(mutex[topic_ID]));
            break;
          }
        }
      // sleep for milliseconds
      } else if(!strcmp(commands[0],"sleep")) {
        char tmp2[6] = {'\0'};
        int time;
        int r = sscanf(line,"%s %d\n",tmp2,&time);
        // create timespec struct with miliseconds entered
        struct timespec timeSleep;
        // Divide to get milli, modulo to get remainder and multiply to get nano
        timeSleep.tv_sec = time / 1000;
        timeSleep.tv_nsec = (time % 1000) * 1000000;
        // nanosleep sleeps for seconds
        nanosleep(&timeSleep,NULL);
        printf("Proxy thread %d- type: Publisher - Executed command: sleep\n",((publish *)args)->threadId);
      } else if(!strcmp(commands[0],"stop")) {
        // this used by cleaner to stop execution
        ((publish *)args)->notRunning = 1;
        // free vars
        fclose(fileC);
        free(line);
        printf("Proxy thread %d- type: Publisher - Executed command: stop\n",((publish *)args)->threadId);
        pthread_exit(NULL);
      }
      // get next line
      characters = getline(&line, &bsize, fileC);
    }
  }
}

void *subscriber(void *args) {
  int returned;
  // for lastEntry
  int lastNum;
  // run till no entries
  while(1){
    pthread_mutex_lock(&thread_lock);
    // While thread removed, wait
    while(((subscribe *)args)->notRunning) { pthread_cond_wait(&cond,&thread_lock); };
    pthread_mutex_unlock(&thread_lock);
    sleep(2);
    if(!strcmp(((publish*)args)->file, "")) { pthread_exit(NULL); };
    // Print sub thread made, start
    printf("Proxy thread: %d - type: Subscriber\n",((subscribe *)args)->threadId);
    sleep(2);
    FILE *fileC;
    fileC = fopen(((subscribe *)args)->file, "r");
    // file not opened
    if(fileC == NULL) { pthread_exit(NULL); };
    // Line parsing below with strcmp, Assistance from brother on parsing files effectively
    char *line = NULL;
    size_t characters;
    size_t bsize;
    characters = getline(&line, &bsize, fileC);
    // create subscriber html file
    char file[MAXTFILE];
    sprintf(file,"subscriber_%d.html",((subscribe *)args)->threadId);
    // open subscriber file for writing
    FILE *output;
    output = fopen(file, "w");
    // Print head of html file
    fprintf(output,"<!DOCTYPE html> <html><head><title>Subscriber %d </title><style>table, th, td {border: 1px solid black;border-collapse: collapse;}th, td {padding: 5px;}th {text-align: left;</style></head><body>",((subscribe *)args)->threadId);
    while(characters != -1) {
      // init commands array
      char commands[1][10] = {0};
      int count = 0;
      for (int i = 0; i < characters;i++) {
        if(line[i]!= ' ') {
          if(line[i]=='\n') { break; };
            commands[0][count] = line[i];
            count++;
        } else { break; };
      }
      if(!strcmp(commands[0],"get")) {
        char tmp[4] = {'\0'};
        int id = 0;
        int r = sscanf(line,"%s %d\n",tmp,&id);
        // find topic queue index that has topic ID = id
        int topic_ID = -1;
        for (int queue = 0; queue < MAXTOPICS ; queue++) {
          if(registry[queue].ID == id) { topic_ID = queue; break; };
        }
        // If a removed  entry, ignore
        if(topic_ID == -1) {
          // nothing
        } else {
          // start the html file
          fprintf(output,"<h1>Subscriber: %d </h1><h2>Topic Name: %s</h2><table style=\"width:100%%\" align=\"middle\"><tr><th>CAPTION</th><th>PHOTO-URL</th></tr>",((subscribe *)args)->threadId,registry[topic_ID].name);
          lastNum = 0;
          // to count MAXTRIES
          int tries = 0;
          while(1) {
            pthread_mutex_lock(&(mutex[topic_ID]));
            // create empty topicEntry
            topicEntry empty;
            returned = getEntry(lastNum,&empty,topic_ID);

            // if queue empty it return 0, done getting
            if(returned == 0) {
              printf("Proxy thread %d- type: Subscriber - Executed command: get\n",((subscribe *)args)->threadId);
              fprintf(output,"</table>");
              // unlocks mutex for queue, and breaks
              pthread_mutex_unlock(&(mutex[topic_ID]));
              break;
            // If didn't get anything, yield each time
            } else if(returned == -1){
              tries++;
              // If maxtries of 4000 reached then exit and enter empty into html, on empty queue
              if(tries >= 4000) {
                printf("Subscriber #%d: Reached attempt limit on empty queue\n", ((subscribe *)args)->threadId);
                pthread_mutex_unlock(&(mutex[topic_ID]));
                break;
              }
              pthread_mutex_unlock(&(mutex[topic_ID]));
              sched_yield();
            // got entry, enter to html, get next at last+1
            } else {
              fprintf(output,"<tr><td>%s</td><td><img src=%s></td></tr>",empty.photoCaption,empty.photoURL);
              // if got entry print, set last to current
              if(returned > 1) {
                lastNum = returned;
              } else { lastNum++; };
              pthread_mutex_unlock(&(mutex[topic_ID]));
            }
          }
      }
      // Call nanosleep to sleep for milliseconds, take the entered int and conver to milliseconds order then sleep, have to account for leftover nanoseconds
      } else if(!strcmp(commands[0],"sleep")) {
        char tmp2[6] = {'\0'};
        int time;
        int r = sscanf(line,"%s %d\n",tmp2,&time);

        struct timespec timeSleep;
        timeSleep.tv_sec = time / 1000;
        timeSleep.tv_nsec = (time % 1000) * 1000000;
        nanosleep(&timeSleep,NULL);
        printf("Proxy thread %d- type: Subscriber - Executed command: sleep\n",((subscribe *)args)->threadId);
      } else if(!strcmp(commands[0],"stop")) {
        // for stopping the cleaner
        ((subscribe *)args)->notRunning = 1;
        // free and close
        fclose(fileC);
        free(line);
        fprintf(output,"</body></html>");
        fclose(output);
        printf("Proxy thread %d- type: Subscriber - Executed command: stop\n",((subscribe *)args)->threadId);
        pthread_exit(NULL);
      }
      characters = getline(&line, &bsize, fileC);
    }
  }
}

void *cleaner(void *args) {
  while(1) {
    // wait for start
    pthread_mutex_lock(&thread_lock);
    while(((cleanIt *)args)->notRunning) { pthread_cond_wait(&cond,&thread_lock); };
    pthread_mutex_unlock(&thread_lock);
    sleep(2);
    // if notRunning is 2 exit, for freeing/joining
    if(((cleanIt *)args)->notRunning == 2) { pthread_exit(NULL); };
    // call dequeue on each queue
    for (int queue = 0; queue < (((cleanIt *)args)->queues); queue++) {
      pthread_mutex_lock(&(mutex[queue]));
      dequeue(queue);
      pthread_mutex_unlock(&(mutex[queue]));
      sched_yield();
    }
  }
}

int main(int argc, char *argv[]) {
  // thread attributes and inits it to default
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  //init mutexes
  for (int k = 0; k < MAXTOPICS; k++) { pthread_mutex_init(&mutex[k],NULL); };
  //create publishers
  for (int i = 0; i < NUMPROXIES/2; i++) {
    // thread gets id and removed = 1 so it doesnt start, then create
    pubargs[i].threadId = i + 1;
    pubargs[i].notRunning = 1;
    pthread_create(&pubs[i],&attr,publisher, (void*) &pubargs[i]);
  }
  //create subscribers
  for (int i = 0; i < NUMPROXIES/2; i++) {
    // thread gets id and removed = 1 so it doesnt start, then create
    subargs[i].threadId = i + 1;
    subargs[i].notRunning = 1;
    pthread_create(&subs[i],&attr,subscriber, (void*) &subargs[i]);
  }
  //create cleaner, gets id and notRunning = 1 so it doesnt start, then create
  pthread_t clean;
  cleaner_args.threadId = 1;
  cleaner_args.notRunning = 1;
  pthread_create(&clean, &attr,cleaner, (void*) &cleaner_args );
  // entries for topicQs
  int num_queues = 0;
  // used for stopping nonutilized threads
  int pub_tasked = 0;
  int sub_tasked = 0;
  // default null
  topicEntry last = { .entryNum = -1, };
  FILE *input;
  input = fopen(argv[1], "r");
  if(input == NULL) { printf("Error! No file: %s.\n", argv[2]); exit(EXIT_FAILURE); };
  // Line parsing below with strcmp, Assistance from brother on parsing files effectively
  char *line = NULL;
  line = (char *)malloc(32 * sizeof(char));
  size_t characters;
  size_t bsize = 32;
  characters = getline(&line, &bsize, input);
  while(characters != -1) {
    char commands[1][10] = {0};
    int count = 0;
    for (int i = 0; i < characters;i++) {
      if(line[i]!= ' '){
        commands[0][count] = line[i];
        count++;
      } else { break; };
    }
    if(!strcmp(commands[0],"create")) {
      char tmp[15] = {'\0'};
      char tmp2[15] = {'\0'};
      int id, len;
      char name[30] = {'\0'};
      int create = sscanf(line,"%s %s %d %s %d", tmp, tmp2,&id,name,&len);
      // If too many topics exit
      if(num_queues >= MAXTOPICS) {
        printf("exceeds MAXTOPICS\n");
        exit(EXIT_FAILURE);
      } else {
        // create topic queue
        strcpy(registry[num_queues].name,name);
        registry[num_queues].buffer = buffers[num_queues];
        registry[num_queues].buffer[0].entryNum  = 0;
        registry[num_queues].length = len;
        registry[num_queues].head = 0;
        registry[num_queues].tail = 0;
        registry[num_queues].entries = 0;
        registry[num_queues].ID = id;
        registry[num_queues].buffer[len] = last;
        // increment num_queues
        num_queues++;
      }
    } else if(!strcmp(commands[0],"add")) {
      char tmp[15] = {'\0'};
      char type[15] = {'\0'};
      char f[MAXTFILE] = {'\0'};
      int add = sscanf(line,"%s %s \"%[^\n\"]\"", tmp, type, f);
      for (int i = 0; i < NUMPROXIES/2; i++) {
        // Find non utlized thread, of publish or subscribe and create
        if(!strcmp(type,"publisher") && pubargs[i].notRunning) {
          strcpy(pubargs[i].file,f);
          pubargs[i].notRunning = 0;
          pub_tasked += 1;
          break;
        } else if(!strcmp(type,"subscriber") && subargs[i].notRunning) {
          strcpy(subargs[i].file,f);
          subargs[i].notRunning = 0;
          sub_tasked += 1;
          break;
        }
      }
    } else if(!strcmp(commands[0],"query")) {
      char tmp[10] = {0};
      char type[15] = {0};
      int query = sscanf(line,"%s %[^\n]",tmp,type);
        // query pubs, show running only
      if(!strcmp(type,"publishers") && pub_tasked) {
        printf("Query Publishers:\n");
        for (int i = 0; i < NUMPROXIES/2;i++) {
          if(!pubargs[i].notRunning) { printf("Publisher proxy thread ID: %d, file name: %s\n",pubargs[i].threadId, pubargs[i].file); };
        }
      // query subs, show running only
      } else if(!strcmp(type,"subscribers")&& sub_tasked){
        printf("Query Subscribers:\n");
        for (int i = 0; i < NUMPROXIES/2;i++) {
          if(!subargs[i].notRunning) { printf("Subscriber proxy thread ID: %d, file name: %s\n",subargs[i].threadId,subargs[i].file); };
        }
      // query topics
      } else if(!strcmp(type,"topics")&&num_queues>0) {
        printf("Query Topics:\n");
        for (int i = 0; i < num_queues; i++) { printf("Topic ID: %d, Name: %s, Topic Length: %d\n",registry[i].ID,registry[i].name,registry[i].length); };
      }
    // set delta
    } else if(!strcmp(commands[0],"delta")) {
      char tmp[10];
      double d;
      int delta = sscanf(line,"%s %lf",tmp,&d);
      DELTA = d;
    } else if(!strcmp(commands[0],"start\n") || !strcmp(commands[0], "start")) {
      sleep(2);
      printf("##### STARTING ALL #####\n");
      cleaner_args.notRunning = 0;
      cleaner_args.queues = num_queues;
      printf("Starting cleaner thread\n");
      sleep(2);
      pthread_mutex_lock(&thread_lock);
      pthread_cond_broadcast(&cond);
      pthread_mutex_unlock(&thread_lock);
      sleep(2);
      break;
    }
    // get next
    characters = getline(&line, &bsize, input);
  }
  // loop till all stopped
  while(1) {
    int stopRunning = 1;
    for (int i = 0; i<NUMPROXIES/2;i++) {
      // if either pub or sub is not notRunning, stopped will negate
      if(!pubargs[i].notRunning || !subargs[i].notRunning ){ stopRunning = 0; };
    }
    // If stopped then set cleaner to exit
    if(stopRunning) { cleaner_args.notRunning = 2; break; };
  }
  // set threads to exit
  for(int i = pub_tasked; i < NUMPROXIES/2; i++) {
    strcpy(pubargs[i].file, "z");
    pubargs[i].notRunning = 0;
  }
  for(int i = sub_tasked; i < NUMPROXIES/2; i++) {
    strcpy(subargs[i].file, "");
    subargs[i].notRunning = 0;
  }
  cleaner_args.notRunning = 2;

  // broadcast changes
  pthread_mutex_lock(&thread_lock);
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&thread_lock);
  sleep(2);
  // join threads
  for(int i = 0; i < NUMPROXIES/2; i++) { pthread_join(pubs[i], NULL); };
  for(int z = 0; z < NUMPROXIES/2; z++) { pthread_join(subs[z], NULL); };
  pthread_join(clean, NULL);
  // print stopped
  printf("##### ALL STOPPED #####\n");
  // free and close
  fclose(input);
  free(line);
  return 1;
}
