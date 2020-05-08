/*
* Description: Project 1 command c file. Contains functions necessary for project.
*
*
* Author: Irfan Filipovic
*
* Date: 04/17/2020
*
* Notes:
* 1. Utilize no
*/
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include "command.h"
#include <dirent.h>
#include <fcntl.h>
// mkdir(2) man page
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <errno.h>

/*for the ls command*/
void listDir() {
    // Vars
    char cwd[FILENAME_MAX];
    char *path;
    DIR *openD;
    struct dirent *file;
    // List files in cwd
    path = getcwd(cwd, sizeof(cwd));
    openD = opendir(path);
    while((file = readdir(openD)) != NULL) {
      write(1, file->d_name, strlen(file->d_name));
      write(1, " ", 1);
    }
    write(1, "\n", 1);
    closedir(openD);
}

/*for the pwd command*/
void showCurrentDir() {
  // Vars
  char cwd[FILENAME_MAX];
  char *path;

  // List files in cwd
  path = getcwd(cwd, sizeof(cwd));
  write(1, path, strlen(path));
  write(1, "\n", 1);
}

/*for the mkdir command*/
void makeDir(char *dirName) {
  // Vars
  int err;
  // make a dir, permissions set read, write, execute for user, group, and another
  // if error mkdir returns -1 and error caught;
  switch((err = mkdir(dirName, 0777))) {
  case -1: write(1, "Error occured!\n", 15); break;
  defualt:        write(1, "mkdir: created directory ", 25);
                  write(1, dirName, strlen(dirName));
                  write(1, "\n", 1);
                  break;
  }
}

/*for the cd command*/
void changeDir(char *dirName) {
  // Vars
  int err;
  // change dir to dirName
  // if error chdir returns -1 and error caught;
  switch((err = chdir(dirName))) {
  case -1: write(1, "Error occured!\n", 15); break;
  default:        write(1, "user: ", 6);
                  showCurrentDir();
                  break;
  }
}

/*for the cp command*/
void copyFile(char *sourcePath, char *destinationPath) {
  // Vars
  int cur, new, err, error, staterror;
  off_t fSize;
  char line;
  off_t *offset = 0;
  // stat(2) linux man page
  // stat_buf information page
  struct stat stat_buf;

  staterror = stat(sourcePath, &stat_buf);
  if( staterror == -1) {
    //"stat error";
    ;
  }
  switch(stat_buf.st_mode & S_IFMT) {
    case S_IFDIR: (error = mkdir(destinationPath, stat_buf.st_mode)) == 1 ? write(1, "Error occured!\n", 15) : (int)error; break;
    default:      (error = link(sourcePath, destinationPath)) == 1 ? write(1, "Error occured!\n", 15) : (int)error; break; // creates a link/copy
  }
}

/*for the mv command*/
void moveFile(char *sourcePath, char *destinationPath) {
    // Vars
    int err;
    // rename from source to destination, if error then catch
    switch(err = rename(sourcePath, destinationPath)) {
      case -1: write(1, "Error occured!\n", 15); break;
      default: break;
    }
  }

 /*for the rm command*/
void deleteFile(char *filename) {
  // Vars
  int err;
  // make a dir, permissions set read, write, execute for user, group, and another
  // if error mkdir returns -1 and error caught;
  switch(err = unlink(filename)) {
    case -1: write(1, "Error occured!\n", 15); break;
    default: break;
  }
}

/*for the cat command*/
void displayFile(char *filename) {
  // Vars
  char *line = NULL;
  size_t len = 0;
  FILE *cur;
  char *word;

  cur = fopen(filename, "r");
  // Read in each line using getline()
  write(1, "\n", 1);
  while(getline(&line, &len, cur) != -1)
  {
    // Write the line to stdout
    word = line;
    write(1, word, strlen(word));
  }
  write(1, "\n", 1);
  fclose(cur);
  free(line);
}
