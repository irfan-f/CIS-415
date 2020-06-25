/*
* Lab 8 CIS 415 University of Oregon
* Author: Irfan Filipovic
* Date: 05/21/20
* NOTES:
*/
#define MAXNAME 32
#define MAXQUEUES 16
#define MAXENTRIES 3
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// MT Struct
struct mealTicket {
  int ticketNum;
  char *dish;
};

// MTQ Struct
struct MTQ {
  char name[MAXNAME];
  struct mealTicket* buffer[MAXENTRIES];
  int head;
  int tail;
  int length;
};

// MTQ REGISTRY ARRAY
struct MTQ *registry[MAXQUEUES];

// ENQUEUE
int enqueue(char *MTQ_ID, struct mealTicket *MT) {
  int pos = -1;
  for(int i = 0; i < MAXQUEUES; i++) {
    if(strcmp(MTQ_ID, registry[i]->name) == 0) {
      pos = i;
      break;
    }
  }
  if (pos == -1) {
    return 0;
  }
  // Will not work, sequential only
  // If head + 1 == Tail
  // For empty, if same and not initialized values
  if((registry[pos]->tail - registry[pos]->head) >= MAXENTRIES) {
    return 0;
  }
  if(((registry[pos]->tail + 1) % MAXENTRIES) == registry[pos]->head)
  MT->ticketNum = registry[pos]->length;
  registry[pos]->buffer[registry[pos]->tail] = MT;
  registry[pos]->tail = (registry[pos]->tail + 1);
  registry[pos]->length = (registry[pos]->length + 1);
  return 1;
}

// DEQUEUE
int dequeue(char *MTQ_ID, int ticketNum, struct mealTicket *MT) {
  int pos = -1;
  for(int i = 0; i < MAXQUEUES; i++) {
    if(strcmp(MTQ_ID, registry[i]->name) == 0) {
      pos = i;
      break;
    }
  }
  if (pos == -1) {
    return 0;
  }
  if(registry[pos]->tail == registry[pos]->head) {
    return 0;
  }
  *MT = *registry[pos]->buffer[registry[pos]->head];
  registry[pos]->buffer[registry[pos]->head] = NULL;
  registry[pos]->head++;
  return 1;
}

void main()
{
  // 1. registry init above, generally do global vars outside of main
  // a. create Breakfast, Lunch, Dinner, and Bar
  struct MTQ *zero = malloc(sizeof(struct MTQ));
  struct MTQ *one = malloc(sizeof(struct MTQ));
  struct MTQ *two = malloc(sizeof(struct MTQ));
  struct MTQ *three = malloc(sizeof(struct MTQ));
  strcpy(zero->name, "Breakfast");
  strcpy(one->name, "Lunch");
  strcpy(two->name, "Dinner");
  strcpy(three->name, "Bar");
  //printf("%s\n", zero->name);
  //printf("%s\n", one->name);
  //printf("%s\n", two->name);
  //printf("%s\n", three->name);
  zero->head = 0;
  zero->tail = 0;
  zero->length = 0;
  one->head = 0;
  one->tail = 0;
  one->length = 0;
  two->head = 0;
  two->tail = 0;
  two->length = 0;
  three->head = 0;
  three->tail = 0;
  three->length = 0;
  registry[0] = zero;
  registry[1] = one;
  registry[2] = two;
  registry[3] = three;
  char *names[4];
  names[0] = "Breakfast";
  names[1] = "Lunch";
  names[2] = "Dinner";
  names[3] = "Bar";
  // 2.
  // Create 3 tickets each and enqueue
  struct mealTicket *bt1 = malloc(sizeof(struct mealTicket));
  bt1->dish = "Cereal";
  bt1->ticketNum = 0;
  if(enqueue("Breakfast", bt1) == 0) { printf("error E\n"); };
  struct mealTicket *bt2 = malloc(sizeof(struct mealTicket));
  bt2->dish = "Orange Juice";
  bt2->ticketNum = 1;
  if(enqueue("Breakfast", bt2) == 0) { printf("error E\n"); };
  struct mealTicket *bt3 = malloc(sizeof(struct mealTicket));
  bt3->dish = "Pancakes";
  bt3->ticketNum = 2;
  if(enqueue("Breakfast", bt3) == 0) { printf("error E\n"); };

  struct mealTicket *lt1 = malloc(sizeof(struct mealTicket));
  lt1->dish = "Pizza";
  lt1->ticketNum = 0;
  if(enqueue("Lunch", lt1) == 0) { printf("error\n"); };
  struct mealTicket *lt2 = malloc(sizeof(struct mealTicket));
  lt2->dish = "Sandwich";
  lt2->ticketNum = 1;
  if(enqueue("Lunch", lt2) == 0) { printf("error\n"); };
  struct mealTicket *lt3 = malloc(sizeof(struct mealTicket));
  lt3->dish = "Soup";
  lt3->ticketNum = 2;
  if(enqueue("Lunch", lt3) == 0) { printf("error\n"); };

  struct mealTicket *dt1 = malloc(sizeof(struct mealTicket));
  dt1->dish = "Steak";
  dt1->ticketNum = 0;
  if(enqueue("Dinner", dt1) == 0) { printf("error\n"); };
  struct mealTicket *dt2 = malloc(sizeof(struct mealTicket));
  dt2->dish = "Mashed Potatoes";
  dt2->ticketNum = 1;
  if(enqueue("Dinner", dt2) == 0) { printf("error\n"); };
  struct mealTicket *dt3 = malloc(sizeof(struct mealTicket));
  dt3->dish = "Ice Cream";
  dt3->ticketNum = 2;
  if(enqueue("Dinner", dt3) == 0) { printf("error\n"); };

  struct mealTicket *t1 = malloc(sizeof(struct mealTicket));
  t1->dish = "Moscow Mule";
  t1->ticketNum = 0;
  if(enqueue("Bar", t1) == 0) { printf("error\n"); };
  struct mealTicket *t2 = malloc(sizeof(struct mealTicket));
  t2->dish = "Long island";
  t2->ticketNum = 1;
  if(enqueue("Bar", t2) == 0) { printf("error\n"); };
  struct mealTicket *t3 = malloc(sizeof(struct mealTicket));
  t3->dish = "Pepsi";
  t3->ticketNum = 2;
  if(enqueue("Bar", t3) == 0) { printf("error\n"); };
  printf("===== Round Robin Dequeue =====\n");
  // 3.
  // first ticket from each
  for(int a = 0; a < 3; a++) {
    // each queue
    for(int i = 0; i < 4; i++) {
      struct mealTicket dqTicket;
      if(dequeue(names[i], 0, &dqTicket) == 0) { printf("error\n"); };
      printf("Queue: %s - Ticket Number: %d - Dish: %s\n", names[i],dqTicket.ticketNum, dqTicket.dish);
    }
  }
  // 4
  printf("===== EN/DE Queue Test Cases =====\n");
  struct MTQ *test = malloc(sizeof(struct MTQ));
  strcpy(test->name, "Test");
  test->tail = 0;
  test->head = 0;
  test->length = 0;
  registry[4] = test;
  printf("     Empty Queue\n");
  // A.
  struct mealTicket test1;
  if(dequeue("Test", 0, &test1) == 0) {
    printf("Test Case: A - Result: Success\n");
  } else {
    printf("Test Case: A - Result: Fail\n");
  }
  // D.
  if(enqueue("Test", t1) == 0) {
    printf("Test Case: D - Result: Fail\n");
  } else {
    printf("Test Case: D - Result: Success\n");
  }
  // Full Queue
  printf("     Full Queue\n");
  if(enqueue("Test", t1) == 0) { printf("error\n"); };
  if(enqueue("Test", t2) == 0) { printf("error\n"); };
  // C.
  if(enqueue("Test", t3) == 0) {
    printf("Test Case: C - Result: Success\n");
  } else {
    printf("Test Case: C - Result: Fail\n");
  }
  // B.
  struct mealTicket test2;
  if(dequeue("Test", 0, &test2) == 0) {
    printf("Test Case: B - Result: Fail\n");
  } else {
    printf("Test Case: B - Result: Success\n");
  }
  free(test);
  // Free All
  free(zero);
  free(one);
  free(two);
  free(three);
  free(bt1);
  free(bt2);
  free(bt3);
  free(lt1);
  free(lt2);
  free(lt3);
  free(dt1);
  free(dt2);
  free(dt3);
  free(t1);
  free(t2);
  free(t3);
}
