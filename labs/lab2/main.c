/*
* Description: Compile code with 'gcc main.c', run with './a.out'. It runs a simple interactive
*              CLI that will tokenize input and return each token. Upon entering exit the program will
*              free memory and terminate process, but no other event can end the process.
*
* Author: Irfan Filipovic
*
* Date: 04/10/20
*
* Notes:
* 1. Code is a little messy, but completes requirements.
*/
/*-------------------------Preprocessor Directives---------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
/*-----------------------------Program Main----------------------------------*/
int main() {
setbuf(stdout, NULL);
/* Main Function Variables */
FILE *stream;
char *line = NULL;
size_t len = 0;
ssize_t nread;
bool exitCall = false;
char *exitToken = "exit";
char *exitToken2 = "exit\n";
stream = stdin;
char *word;
char *subWord;

char *subToken;
char *token;
int count = 0;
/* Allocate memory for the input buffer. */
/*main run loop*/
while(!exitCall) {
/* Print >>> then get the input string */
  printf(">>> ");
  getline(&line, &len, stream);
  /* Tokenize the input string */
  word = line;
  token = strtok_r(word, ";", &word);
  // On empty line simply rerun loop
  int same = strcmp(token, " ");
  if( same == -22 || same == 32 || same == 10) {
    continue;
  };
  printf("\n");
  while (token != NULL) {
    if(count != 0) {
      printf("T%d: ;\n", count);
      count++;
    }
    subWord = token;
    subToken = strtok_r(subWord, " ", &subWord);
    while(subToken != NULL) {
      // Exit call does not work as not expecting exit call in file, reads EOF.
      int exitVal = strcmp(subToken, exitToken);
      int exitVal2 = strcmp(subToken, exitToken2);
      if(exitVal == 0 || exitVal2 == 0) {
        exitCall = true;
        /*Free the allocated memory*/
        printf("\n");
        free(line);
        fclose(stream);
        return 1;
      }
      subToken = strtok_r(NULL, " ", &subWord);
    }
    // Remove leading whitespace, increment i each whitespace character leading
    // add i to start of token to ignore white spaces
    // Print token without leading, increment count
    int i;
    for (i = 0; token[i] == ' '; i++);
    token = token + i;
      /* Display each token */
    printf("T%d: %s\n", count, token);
    count++;
    token = strtok_r(NULL, ";", &word);
  }
  // set count of tokens to 0 on new getline()
  count = 0;
}
}
/*-----------------------------Program End-----------------------------------*/
