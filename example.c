#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

void writeUnique(int fd){
  write(fd, "uniq -ic allwords_sorted.txt count_uniqwords.txt&\nbarrier\n", strlen("uniq -ic allwords_sorted.txt count_uniqwords.txt&\nbarrier\n"));
}

void writeSort(int fd){
  write(fd, "sort -o allwords_sorted.txt allwords.txt&\nbarrier\n", strlen("sort -o allwords_sorted.txt allwords.txt&\nbarrier\n"));
}

/*===========================================================================*/
/* This function writes the grep command in the file
/*===========================================================================*/
void writeGrep(int fd){
  char letters[2];
  letters[0] = 'A';
  letters[1] = 'A';

  char * command = "grep -oh [a-zA-Z]* ";
  write(fd, command,  strlen(command));
  for(int i = 0; i < 26; i++){
    for(int j = 0; j < 26; j++){
      char * file = (char *)malloc(7);
      sprintf(file, "%s.txt ", letters);
      write(fd, file, strlen(file));
      letters[1] ++;
    }
    letters[0]++;
    letters[1] = 'A';
  }
  write(fd, "> allwords.txt&\nbarrier\n", strlen("> allwords.txt&\nbarrier\n"));
}

/*===========================================================================*/
/* This function writes the html2text command for every file
/*===========================================================================*/
void extractText(int fd){
  int barrier = 0;
  char letters[2];
  letters[0] = 'A';
  letters[1] = 'A';

  for(int i = 0; i < 26; i++){
    for(int j = 0; j < 26; j++){
      char * command = (char *)malloc(38);
      sprintf(command, "lynx -dump -nolist %s.html > %s.txt&\n", letters, letters);
      int nr = write(fd, command, strlen(command));
      letters[1]++;
      barrier++;
      if(barrier == 10){
        write(fd, "barrier\n", 8);
        barrier = 0;
      }
    }
    letters[0]++;
    letters[1] = 'A';
  }
  write(fd, "barrier\n", 8);
}

/*===========================================================================*/
/* This function writes every wget to the batch file
/*===========================================================================*/
int writePageCommands(int fd){
  int barrier = 0;
  char letters[2];
  letters[0] = 'A';
  letters[1] = 'A';

  for(int i = 0; i < 26; i++){
    for(int j = 0; j < 26; j++){
      char * command = (char *)malloc(47);
      sprintf(command, "wget https://wikipedia.org/wiki/%s -O %s.html&\n", letters, letters);
      int nr = write(fd, command, strlen(command));
      if(nr == -1){
        printf("Error writing to the file\n");
        return 1;
      }
      letters[1] ++;
      barrier++;
      if(barrier == 10){
        write(fd, "barrier\n", 8);
        barrier = 0;
      }
    }
    letters[0]++;
    letters[1] = 'A';
  }
  int nr = write(fd, "barrier\n", 8);
  return 0;
}

int main(int argc, char * argv[]){

  if(argc != 2){
    printf("Invalid number of arguments\n");
    return 0;
  }

  char * batchfile = argv[1];
  int fd = open(batchfile, O_CREAT | O_WRONLY, 0644);
  if(fd == -1){
    printf("Cannot open file %s\n", batchfile);
    return 0;
  }

  writePageCommands(fd);
  extractText(fd);
  writeGrep(fd);
  writeSort(fd);
  writeUnique(fd);
  write(fd, "sort -k 1,1n count_uniqwords.txt&\nbarrier\nquit\n", strlen("sort -k 1,1n count_uniqwords.txt&\nbarrier\nquit\n"));
  return 0;
}
