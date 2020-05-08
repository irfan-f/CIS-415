/*
* Description: Project1 provides functionalities of rm, ls, cd, cp , mv, cat, pwd in a basic shell program.
*
* Author: Irfan Filipovic
*
* Date: 04/17/20
*
* Notes:
* 1. Only bug is cp <arg1> <arg2> ; <cmd> ...
*		 Will not execute following commands after cp, bug is currently being looked into.
*		 Error occurs as variable 'token' is altered to represent a substring of token.
*		 Did not find fix before needing to turn in assignment.
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
	FILE *outstream;
	char *line = NULL;
	size_t len = 0;
	char *word;
	char *subWord;
	char *token;
	char *subToken;
	char *command;
	int flagCatch = 0;
	char *flagVar = "-f";
	int bad;
	stream = stdin; // Can alter to read in from file
	outstream = stdout;
	char *accepted[] = {"ls\0", "ls \0", " ls\0", "exit \0", "exit\0", " exit\0", "pwd\0", "pwd \0", " pwd\0", "mkdir\0", "mkdir \0", " mkdir\0", "cd\0", "cd \0", " cd\0", "cp\0", "cp \0", " cp\0", "mv\0", "mv \0", " mv\0", "rm\0", "rm \0", " rm\0", "cat\0", "cat \0", " cat\0"};

	int dirFlag = '/';
	char *dirStr = "/";
	char *curFlag = ".";
	/* Switch the context of STDOUT if the program is started with the -f flag.
	   Open the file for reading and open a file "output.txt" for writing.
	   Hint: use freopen() to switch the context of a file descriptor. */
	if (argc > 1) {
		if((strcmp(argv[1], flagVar) == 0)) {
		 	flagCatch = 1;
		}
	}

	// Main run cycle
	do {
		// Display prompt and read input from console/file using getline(3)
		if (flagCatch == 0)
		{
			printf(">>> ");
			getline(&line, &len, stream);
			// Tokenize the input string
			word = line;
			token = strtok_r(word, ";", &word);
			// empty line rerun
			int same = strcmp(token, " ");
			if(same == -22 || same == 32 || same == 10) {
				continue;
			}
		} else {
			if(flagCatch == 1){
			 	int lenCheck = strlen(argv[2]);
				char * tmpStr;
				argv[2][lenCheck] = '\0';
				tmpStr = argv[2];
				outstream = freopen("output.txt", "w", stdout);
				stream = fopen(tmpStr, "r");
				if(getline(&line, &len, stream) == -1) {
					condition = 0;
				} else {
					word = line;
					token = strtok_r(word, ";", &word);
				}
			} else {
				if(getline(&line, &len, stream) == -1) {
					condition = 0;
				} else {
					word = line;
					token = strtok_r(word, ";", &word);
				}
			}
		}
		if(flagCatch == 1) { flagCatch = 2;};
		int count = 0;
		while (token != NULL)
		{
			size_t tokenLen = strcspn(token, "\n");
			token[tokenLen] = '\0';
			printf("token is: %s\n", token);
			subWord = token;
			subToken = strtok_r(subWord, " ", &subWord);
			while(subToken != NULL) {
				int match;
				int i = 0;
				int answer = 100;
				for(i; i < 27; i++) {
					match = strcmp(token, accepted[i]);
					if(match == 0) {
						answer = i;
						break;
					}
				}
				printf("token is: %s\n", token);
				if (count == 0)
				{
					if (answer == 0 || answer == 1 || answer == 2) {
							// ls
							command = "ls";
							//printf("This cmd entered: %s\n", command);
							char *arg1;
							arg1 = strtok_r(NULL, " ", &subWord);
								// keep track of arg, if one then error
							if(arg1 != NULL) {
								if(strcmp(arg1, command) != 0) {
									write(1, "Error! Unsupported parameters for command: ls\n", 46);
								} else {
									write(1, "Error! Incorrect syntax. No control code found.\n", 48);
								}
							} else {
								listDir();
							}
					} else if (answer == 3 || answer == 4 || answer == 5) {
						// Exit
						command = "exit";
						char *arg1;
						arg1 = strtok_r(NULL, " ", &subWord);
							// keep track of arg, if one then error
						if(arg1 != NULL) {
							write(1, "Error! Unsupported parameters for command: exit\n", 48);
						} else {
							condition = 0;
						}
					} else if (answer == 6 || answer == 7 || answer == 8) {
						// pwd
						command = "pwd";
						//printf("This cmd entered: %s\n", command);
						char *arg1;
						arg1 = strtok_r(NULL, " ", &subWord);
							// keep track of arg, if one then error
						if(arg1 != NULL) {
							if(strcmp(arg1, command) != 0) {
								write(1, "Error! Unsupported parameters for command: pwd\n", 47);
							} else {
								write(1, "Error! Incorrect syntax. No control code found.\n", 48);
							}
						} else {
							showCurrentDir();
						}
					} else if (answer == 9 || answer == 10 || answer == 11) {
						// mkdir
						command = "mkdir";
						//printf("This cmd entered: %s\n", command);
						char *arg1;
						arg1 = strtok_r(NULL, " ", &subWord);
							// keep track of arg, if another then error
						if((strtok_r(subWord, " ", &subWord)) != NULL) {
							write(1, "Error! Unsupported parameters for command: mkdir\n", 49);
						} else {
							/* // Can implement error checking for ending dirs with '/' char using strrchr and doing index calculations //
							// Not expanded upon as not given such examples in sample
							// Implemented for copy
							*/
							makeDir(arg1);
						}
					} else if (answer == 12 || answer == 13 || answer == 14) {
					// cd
					command = "cd";
					//printf("This cmd entered: %s\n", command);
					char *arg1;
					arg1 = strtok_r(NULL, " ", &subWord);
					printf("token: %s\n", token);

						// keep track of arg, if another then error
					if((strtok_r(subWord, " ", &subWord)) != NULL) {
						write(1, "Error! Unsupported parameters for command: cd\n", 46);
					} else {
						changeDir(arg1);
					}
					} else if (answer == 15 || answer == 16 || answer == 17) {
					// cp


					///// Error occurs because token variable is altered somehow, did not find reason yet
					command = "cp";
					char *arg1, *arg2, *arg3;
					int a1, ind, j;
					char *tmp, *arg1File, *arg2Fix;

					arg1 = strtok_r(NULL, " ", &subWord);
					arg2 = strtok_r(subWord, " ", &subWord);
					arg3 = strtok_r(subWord, " ", &subWord);

					if(arg3 != NULL) {
						write(1, "Error! Unsupported parameters for command: cp\n", 46);
						token = NULL;
						break;
					}


					if ((strrchr(arg1, '/')) != NULL) {
						arg1File = (char *)malloc(sizeof(char) * strlen(arg1));
						a1 = strlen(arg1);
						int i = a1 - 1;
						while(arg1[i] != dirFlag) {
							i--;
						}
						j = 0;
						for(i; i < a1-1; i++) {
							arg1File[j] = arg1[i+1];
							j++;
						}
						arg1File[j] = '\0';
					} else {
						arg1File = arg1;
					}

					if(arg2[(strlen(arg2)-1)] == dirFlag) {
						// add file name to dir path
						arg2Fix = strcat(arg2, arg1File);
					} else if((strcmp(arg2, curFlag)) == 0) {
						// add / and file name to cur dir path
						tmp = strcat(arg2, dirStr);
						arg2Fix = strcat(tmp, arg1File);
					} else {
						arg2Fix = arg2;
					}
					if ((strrchr(arg1, '/')) != NULL) {
						free(arg1File);
					}
					copyFile(arg1, arg2Fix);
					} else if (answer == 18 || answer == 19 || answer == 20) {
					// mv
					command = "mv";
					char *arg1, *arg2;
					arg1 = strtok_r(NULL, " ", &subWord);
					arg2 = strtok_r(subWord, " ", &subWord);
						// keep track of arg, if another then error
					if((strtok_r(subWord, " ", &subWord)) != NULL) {
						write(1, "Error! Unsupported parameters for command: cp\n", 46);
					} else {
						moveFile(arg1, arg2);
					}
					} else if (answer == 21 || answer == 22 || answer == 23) {
					// rm
					command = "rm";
					char *arg1;
					arg1 = strtok_r(NULL, " ", &subWord);
						// keep track of arg, if another then error
					if((strtok_r(subWord, " ", &subWord)) != NULL) {
						write(1, "Error! Unsupported parameters for command: rm\n", 46);
					} else {
						deleteFile(arg1);
					}
					} else if (answer == 24 || answer == 25 || answer == 26) {
					// cat
					command = "cat";
					char *arg1;
					arg1 = strtok_r(NULL, " ", &subWord);
						// keep track of arg, if another then error
					if((strtok_r(subWord, " ", &subWord)) != NULL) {
						write(1, "Error! Unsupported parameters for command: rm\n", 46);
					} else {
						displayFile(arg1);
					}
					} else {
						// Unknown
						printf("Error! Unrecognized command: %s\n", token);
					}
				}
				subToken = strtok_r(NULL, " ", &subWord);
				// if cmd, else nothing, open avenue for more implementation
				if(subToken != NULL && subToken != "") {
					count++;
				}
			}
			token = strtok_r(NULL, ";", &word);
			printf("next token: %s\n", token);
		}

	} while(condition);

	/*Free the allocated memory and close any open files*/
	free(line);
	fclose(stream);

	return 0;
}
/*-----------------------------Program End-----------------------------------*/
