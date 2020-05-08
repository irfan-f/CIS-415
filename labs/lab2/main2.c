/*
* Description: Compile code with 'gcc main.c', run with './a.out'. It runs a simple file mode
*              CLI that will tokenize input and return each token. Upon entering exit the program will
*              free memory and terminate process, but will also free and terminate on end of file.
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
int main(int argc, char *argv[]) {
setbuf(stdout, NULL);

// Simple error check, make sure a file was given when calling program.
if(argc != 2) {
  fprintf(stderr, "Usage: %s <file>\n", argv[0]);
  exit(EXIT_FAILURE);
}
/* Main Function Variables */
FILE *stream;
char *line = NULL;
size_t len = 0;
ssize_t nread;
bool exitCall = false;
char *exitToken = "exit";
stream = fopen(argv[1], "r");
char *word;
char *subWord;

FILE *out = fopen("output.txt", "w");


char *token;
char *subToken;
int count = 0;
/*main run loop*/
while(getline(&line, &len, stream) != -1) {
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
      fprintf(out, "T%d: ;\n", count);
      count++;
    }
    subWord = token;
    subToken = strtok_r(subWord, " ", &subWord);
    while(subToken != NULL) {
      // Exit call does not work as not expecting exit call in file, reads EOF.
      int exitVal = strcmp(subToken, exitToken);
      if(exitVal == 0) {
        exitCall = true;
        /*Free the allocated memory*/
        printf("\n");
        free(line);
        fclose(stream);
        fclose(out);
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
    fprintf(out, "T%d: %s\n", count, token);
    count++;
    token = strtok_r(NULL, ";", &word);
  }
  // set count of tokens to 0 on new getline()
  count = 0;
}
/*Free the allocated memory*/
free(line);
fclose(stream);
fclose(out);
return 1;
}
/*--------------------Program End-----------------------------------*/
