#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include "command.h"
#include <dirent.h>
#include <fcntl.h>

void lfcat()
{
	// Define your variables here
  // Max length allocate
  char cwd[FILENAME_MAX];
  char *path;
  DIR *openD;
  char *line = NULL;
  size_t len = 0;
  char *word;
  char *dot;
  FILE *cur;
  char dashes[80];
  char *format = "File: ";
  char *fileTmp;
  struct dirent *file;

  // create 80 dashes
  for (int i = 0; i < 80; i++) {
    if(i == 79) {
      dashes[i] = '\n';
    } else {
      dashes[i] = '-';
    }
  }
	// Get the current directory
  path = getcwd(cwd, sizeof(cwd));
	// Open the dir using opendir()
  openD = opendir(path);
	// use a while loop to read the dir
  while((file = readdir(openD)) != NULL) {
		// Hint: use an if statement to skip any names that are not readable files (e.g. ".", "..", "main.c", "a.out", "output.txt"

    dot = strrchr(file->d_name, '.'); // sttrchr(3) last occurrence of 2nd arg
    // Check for my .h, .c, output, . dir, .. dir and ignore
    if(dot && (!strcmp(dot, ".h") || !strcmp(dot, ".o") || !strcmp(dot, ".c") || !strcmp(file->d_name, "output.txt") || !strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))) {
      ;
    } else if((!strcmp(file->d_name, "Sampleoutput.txt")) || (!strcmp(file->d_name, "makefile")) || (!strcmp(file->d_name, "a.out")) || (!strcmp(file->d_name, "log.txt"))) {
      ;
    } else {
      write(1, format, strlen(format));
      write(1, strcat(file->d_name, "\n"), strlen(file->d_name)+1); // strcat(3) combine 2 strings // write with (1,) writes to stdout
			// Open the file
      fileTmp = file->d_name;
      int lenTest = strlen(file->d_name);
      fileTmp[lenTest-1] = '\0';
      cur = fopen(fileTmp, "rd");
			// Read in each line using getline()
      while(getline(&line, &len, cur) != -1)
      {
				// Write the line to stdout
        word = line;
        write(1, word, strlen(word));
      }
			// write 80 "-" characters to stdout
      write(1, "\n", 1);
      write(1, dashes, 80);
      // close the read file and free/null assign your line buffer
      fclose(cur);
    }
      //fclose(cur);
    }
	//close the directory you were reading from using closedir()
  free(line);
  closedir(openD);
}
