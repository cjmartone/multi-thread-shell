#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/*===========================================================================*/
/* This function returns the number of digits in n
/*===========================================================================*/
int sizeInt(int n){
  if(n < 10){
    return 1;
  }
  return 1 + sizeInt(n/10);
}

/*===========================================================================*/
/* This function takes a string and puts the substring into sub
/*===========================================================================*/
void substring(char * s, char * sub, int start, int end){
  int c = 0;
  while(c < end){
    sub[c] = s[start+c];
    c++;
  }
  sub[c] = '\0';
}

/*===========================================================================*/
/* This function prints out all contents of the array of strings
/*===========================================================================*/
void printCommands(char ** commands){
  int i = 0;
  do{
    if(commands[i+1] != 0){
      printf("%s ", commands[i]);
    }
    else{
      printf("%s\n", commands[i]);
    }
    i++;
  } while(commands[i] != 0);
}

/*===========================================================================*/
/* This function returns the number of elements in the array of strings
/*===========================================================================*/
int getNumElements(char ** array){
  if(array == 0){
    return 0;
  }

  int i = 0;
  char * str = array[i];
  while(str != 0){
    i++;
    str = array[i];
  }
  return i;
}

/*===========================================================================*/
/* This file creates a child process to execute the command given. */
/* It will return 1 if the exit command is specified. */
/*===========================================================================*/
void execChild(char ** commands, int fd[2]){

  int comLen = getNumElements(commands);
  int lastComLen = strlen(commands[comLen-1]);
  int hasAmp = 0;
  if(strcmp(&(commands[comLen-1][lastComLen-1]), "&") == 0){
    hasAmp = 1;
  }

  pid_t child = fork();
  if(child == 0){
    if(strcmp(commands[0], "quit") != 0 && strcmp(commands[0], "quit&") != 0){
      if(strcmp(commands[0], "barrier") == 0 || strcmp(commands[0], "barrier&") == 0){
        char size[1];
        snprintf(size, 4, "%d", 7);
        write(fd[1], size, 1);
        write(fd[1], "barrier", 7);
        exit(0);
      }

      if(strcmp(&(commands[comLen-1][lastComLen-1]), "&") == 0){
        //check for > pathname
        if(comLen >= 2 && strcmp(commands[comLen-2], ">") == 0){
          char * noAmp = (char *)malloc(strlen(commands[comLen-1]));
          substring(commands[comLen-1], noAmp, 0, lastComLen-1);
          char * path = noAmp;
          int file = open(path, O_CREAT | O_RDWR, 0644);
          dup2(file, 1);
          commands[comLen-1] = 0;
          commands[comLen-2] = 0;

          int pid = (int)getpid();
          int pidLen = sizeInt(pid);
          char size[1];
          snprintf(size, 4, "%d", pidLen);
          write(fd[1], size, 1);
          char pidS[pidLen];
          snprintf(pidS, pidLen+1, "%d", pid);
          write(fd[1], pidS, pidLen);
        }
        else{
          char * noAmp = (char *)malloc(strlen(commands[comLen-1]));
          substring(commands[comLen-1], noAmp, 0, lastComLen-1);
          commands[comLen-1] = noAmp;
          int pid = (int)getpid();
          int pidLen = sizeInt(pid);
          char size[1];
          snprintf(size, 4, "%d", pidLen);
          write(fd[1], size, 1);
          char pidS[pidLen];
          snprintf(pidS, pidLen+1, "%d", pid);
          write(fd[1], pidS, pidLen);
        }
      }
      else{
        if(comLen >= 2 && strcmp(commands[comLen-2], ">") == 0){
          char path[lastComLen];
          strcpy(path, commands[comLen-1]);
          int file = open(path, O_CREAT | O_RDWR, 0644);
          dup2(file, 1);
          commands[comLen-1] = 0;
          commands[comLen-2] = 0;
        }

        char size[1];
        snprintf(size, 4, "%d", 0);
        write(fd[1], size, 1);
      }
      execvp(commands[0], commands);
      fprintf(stderr, " -- Not a valid command\n");
    }
    else{
      char size[1];
      snprintf(size, 4, "%d", 0);
      write(fd[1], size, 1);
    }
    exit(0);
  }
  else if(child < 0){
    printf("Could not fork\n");
  }

  int status;
  if(hasAmp == 0){
    waitpid(child, &status, 0);
  }
}

/*===========================================================================*/
/* This function takes a string and splits it */
/*===========================================================================*/
char ** parseLine(char * line){
  char ** commands = (char **)malloc(sizeof(char **));
  char * split = strtok(line, " \n");

  int i = 0;
  while(split){
    commands = realloc(commands, sizeof(char *) * ++i);
    commands[i-1] = split;
    split = strtok(NULL, " \n");
  }
  if(i > 0){
    commands = realloc(commands, sizeof(char *) * (i+1));
    commands[i] = NULL;
    return commands;
  }
  return 0;
}

/*===========================================================================*/
/* This function takes an open file and returns the next line in it */
/*===========================================================================*/
char * readFile(int fd){
  char buffer[1];
  char * line = (char *)malloc(sizeof(char *));
  ssize_t nr = read(fd, buffer, 1);

  if(nr == 0){ //Empty file
    return 0;
  }
  else if(strcmp(buffer, "\n") == 0){ //Empty line
      return "\n";
  }

  do{
    if(strcmp(buffer, "\n") == 0){
      return line;
    }
    line = realloc(line, strlen(line) + strlen(buffer) + 1);
    strcat(line, buffer);
    line[(int)strlen(line)] = '\0';
  } while((nr = read(fd, buffer, 1)) > 0);

  return line;
}

/*===========================================================================*/
/* This function prompts the user for a command and returns it
/*===========================================================================*/
char * promptUser(){
  printf("prompt> ");
  char * c = malloc(2);
  c[0] = getchar();
  c[1] = 0;

  if(strcmp(c, "\n") == 0){
    return 0;
  }

  char * command = (char *)malloc(sizeof(char *));
  while(strcmp(c, "\n") != 0){
    int len = strlen(command) + 2;
    command = realloc(command, len);
    command[len-2] = c[0];
    command[len-1] = 0;
    c[0] = getchar();
  }
  return command;
}

/*===========================================================================*/
/* Main
/*===========================================================================*/
int main(int argc, char * argv[]){

  if(argc > 2){
    printf("Invalid number of arguments\n");
    return 0;
  }

  char * line = (char *)malloc(sizeof(char *));
  char ** commands = (char **)malloc(sizeof(char **));
  char ** pids = (char **)malloc(sizeof(char **));
  int stdout = dup(1);
  int numpids = 0;
  int fd1[2];
  pipe(fd1);

  if(argc == 1){ /* Interactive mode */

    do{
      line = promptUser();
      char lenToRead[2];
      snprintf(lenToRead, 4, "%d", 1);

      if(line != 0){
        commands = parseLine(line);
        if(commands != 0){
          execChild(commands, fd1);
          read(fd1[0], lenToRead, 1);
          if(atoi(lenToRead) > 0){
            char * pid = (char *)malloc(atoi(lenToRead));
            read(fd1[0], pid, atoi(lenToRead));

            if(strcmp(pid, "barrier") == 0){ //wait for all background processes
              int i = 0;
              for(i; i < numpids; i++){
                int status;
                waitpid(atoi(pids[i]), &status, 0);
              }
              pids = 0;
              numpids = 0;
            }
            else{
              numpids ++;
              pids = realloc(pids, sizeof(char *) * numpids);
              pids[numpids-1] = pid;
            }
          }
        }
      }
    } while(line == 0 || (commands == 0 || strcmp(commands[0], "quit") != 0));
  }

  else{ /* Batch mode */
    // Open the file
    int fd = open(argv[1], O_RDONLY);
    if(fd == -1){
      printf("Cannot open file %s\n", argv[1]);
      return 0;
    }
    // Read the file line by line
    do{
      line = readFile(fd);

      char lenToRead[1];
      snprintf(lenToRead, 4, "%d", 1);

      if(line != 0){
        commands = parseLine(line);

        if(commands != 0){
          printCommands(commands); //Echo command
          execChild(commands, fd1); //Execute command
          read(fd1[0], lenToRead, 1); //Read in the length needed to read

          if(atoi(lenToRead) > 0){
            char * pid = (char *)malloc(atoi(lenToRead)); //make a char array for the pid
            read(fd1[0], pid, atoi(lenToRead)); //read the pid sent from the child

            if(strcmp(pid, "barrier") == 0){ //Wait for all background processes
              dup2(stdout, 1);
              int i = 0;
              for(i; i < numpids; i++){
                int status;
                waitpid(atoi(pids[i]), &status, 0);
              }
              pids = 0;
              numpids = 0;
            }
            else{ //Add pid to the list
              numpids ++;
              pids = realloc(pids, sizeof(char *) * numpids);
              pids[numpids-1] = pid;
            }
          }
        }
      }
    } while(line != 0 && (commands == 0 || strcmp(commands[0], "quit") != 0));
  }

  return 0;
}
