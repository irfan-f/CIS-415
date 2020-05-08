/*
* Description: <write a brief description of your lab>
*
* Author: Irfan Filipovic
*
* Date: 04/17/20
*
* Notes:
* 1. <add notes we should consider when grading>
*/

/*-------------------------Preprocessor Directives---------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include "command.h"
/*---------------------------------------------------------------------------*/

/*-----------------------------Program Main----------------------------------*/
int main(int argc, char *argv[]) {
	setbuf(stdout, NULL);
	// Variable declarations
	int condition = 1;
	char *input;

		// Added a -f flag
	char *flag = "-f";

		// Reading input from user
	FILE *stream;
	FILE *outStream;
	char *line = NULL;
	size_t len = 0;
	char *word;
	char *subWord;
	char *token;
	char *subToken;
	char *command;
	int store = 0;
	int tooLong = 0;
	int flagCatch = 0;
	stream = stdin; // Can alter to read in from file
	outStream = stdout;
	char *accepted[] = {"lfcat\0", "lfcat \0", " lfcat\0" "exit \0", "exit\0"};

	/* Switch the context of STDOUT if the program is started with the -f flag.
	   Open the file for reading and open a file "output.txt" for writing.
	   Hint: use freopen() to switch the context of a file descriptor. */
		 if (argc > 1) {
			 	flagCatch = 1;
		}
		 // Remove the newline at the end of the input string.
	 	//Hint: use strcspn(2) to find where the newline is then assign '\0' there.
	// Main run cycle
	do {
		// Display prompt and read input from console/file using getline(3)
		if (flagCatch == 0)
		{
			printf(">>> ");
			getline(&line, &len, stream);
			tooLong = 0;
			// Tokenize
			word = line;
			token = strtok_r(word, ";", &word);
			// empty line rerun
			int same = strcmp(token, " ");
			if(same == -22 || same == 32 || same == 10) {
				continue;
			}
			int count = 0;
			while (token != NULL)
			{
				// Remove white space leading
				if(token[0] == ' ') {
					token = strcpy(&token[0], &token[1]);
				}
				size_t tokenLen = strcspn(token, "\n");
				token[tokenLen] = '\0';
				if(strlen(token) > 8) {
					tooLong = 1;
				}
				subWord = token;
				subToken = strtok_r(subWord, " ", &subWord);
				while(subToken != NULL) {
					// Exit call does not work as not expecting exit call in file, reads EOF.
					int match;
					int i = 0;
					int answer = 10;
					for(i; i < 4; i++) {
						match = strcmp(token, accepted[i]);
						if(match == 0) {
							answer = i;
							break;
						}
					}
					if (count == 0)
					{
						if (answer == 0 || answer == 1) {
							// lfcat
							command = "lfcat";
							if(tooLong == 1) {
								store = 1;
							} else {
								lfcat();
							}
						} else if (answer == 2 || answer == 3) {
							// Exit
							condition = 0;
						} else {
							// Unknown
							printf("Error! Unrecognized command: %s\n", token);
							store = 99;
						}
					} else {
						// Arguments for lfcat
						if(store == 1) {
							// If valid call then use args
							//printf("arguments parsed, but no args needed\n");
						}
						if (store != 99){
							printf("Error! Unsupported parameters for command: %s\n", command);
						}
					}
					subToken = strtok_r(NULL, " ", &subWord);
					// keep track if command or arg
					if(subToken != NULL) {
						count++;
					}
				}
				token = strtok_r(NULL, ";", &word);
			}
		} else {
			int lenCheck = strlen(argv[2]);
			char * tmpStr;
			argv[2][lenCheck] = '\0';
			tmpStr = argv[2];
			freopen("output.txt", "w", stdout);
			stream = fopen(tmpStr, "r");
			int once;
			once = 1;
			while(getline(&line, &len, stream) != -1) {
			  // Tokenize the input string
				word = line;
				if(once == 1)
				{
					write(1, "File: ", 6);
					write(1, tmpStr, strlen(tmpStr));
					write(1, "\n", 1);
					once = 0;
				}
			  write(1, word, strlen(word));
		}
		write(1, "\n", 1);
		condition = 0;
	}
	} while(condition);

	/*Free the allocated memory and close any open files*/
	free(line);
	fclose(stream);
	return 0;
}
/*-----------------------------Program End-----------------------------------*/
