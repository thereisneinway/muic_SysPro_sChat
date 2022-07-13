#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#define MEM_SIZE 4096
struct shm_st{
  int written,tag;
  char data[BUFSIZ];
};

void quit(int signal) { exit(EXIT_SUCCESS); }

int main(int argc, char *argv[]){
  //ARGUMENT CHECKING AND ASSIGNMENT
  argv++;
  int index = atoi(*argv);
  if(index>2 || index<1){
    fprintf(stderr, "Incorrect input <[1,2]>\n");
    exit(EXIT_FAILURE);
  }
  
  //MEMORY SHARING CHECK
  int running = 1, shmID;
  void *sh_mem = NULL;
  struct shm_st *sh_area;
  shmID = shmget((key_t) 21930, MEM_SIZE,0666 | IPC_CREAT);
  sh_mem = shmat(shmID, NULL, 0);
  if (shmID == -1){
    fprintf(stderr, "shmget failed\n");
    exit(EXIT_FAILURE);
  }
  if (sh_mem == (void *) -1){
    fprintf(stderr, "shmat failed\n");
    exit(EXIT_FAILURE);
  }
  printf("Memory attached at %X\n", sh_mem);//B
  sh_area = (struct shm_st *) sh_mem;//B

  //PROCESS DIVIDER
  pid_t pid = fork();

  //SIGNAL HANDLING
  struct sigaction sg;
  sg.sa_handler = quit;
  sigemptyset(&sg.sa_mask);
  sg.sa_flags = 0;
  sigaction(SIGTERM, &sg, 0);

  //ACUTAL RUNNING LOOP
  switch(pid){
    case -1: exit(EXIT_FAILURE);
    case 0:{
      char buffer[BUFSIZ];
      while (running){
        while (sh_area->written){
          usleep(rand() %4);
        }
        fgets(buffer, BUFSIZ, stdin);
        strcpy(sh_area->data, buffer);
        sh_area->written = 2;
        sh_area->tag = index;
        if (strncmp(buffer, "end chat", 8) == 0){ 
          running = 0;
          kill(getppid(), SIGTERM); 
      }}
      break;
    }
    default:{
      if(index == 2) index = 1;
      else index = 2;
      sh_area->written = 0;
      while (running){
        if (sh_area->written){
          if(sh_area->tag == index){
            printf("Receieve data tag %d: %s",index, sh_area-> data);
            sh_area->written = 0;
          }
          if (strncmp(sh_area->data, "end chat", 8) == 0){
            running = 0;
            kill(0, SIGTERM);
        }}
        usleep(rand() %4);
      }
      break;
   }}//End switch
    
  if (shmdt(sh_mem) == -1 || shmctl(shmID, IPC_RMID, 0) == -1){
    fprintf(stderr, "shmdt/shmctl failed\n");
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
  
}