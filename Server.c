#include <string.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<netinet/in.h>
#include<netdb.h>

#define PORTNUM  1109 /* the port number the server will listen to*/
#define DEFAULT_PROTOCOL 0  /*constant for default protocol*/
#define ARR_SIZE 4

#define SEMKEY ((key_t) 400L)
#define SHMKEY ((key_t) 7890) 
#define NSEMS 1  /*defines the number of semaphores to be created*/

typedef struct{
	char gameArr[4][4];
	int numSockets;
	int numReady;
	int gameStart;
} shared_mem;

//Global Variable Declarations
shared_mem *game;

//const char *ready = "ready";

void doprocessing (int sock);
void setArray ();
char* getArrayStr ();

int main( int argc, char *argv[] ) {
	/*Establish Shared Memory*/
   key_t key = 123; /* shared memory key */ 
   int   shmid; 
   char *shmadd;
   shmadd = (char *) 0; 
   
   if ((shmid = shmget (SHMKEY, sizeof(int), IPC_CREAT | 0666)) < 0)
	{ 
		perror ("shmget"); 
		exit (1); 
	} 

	if ((game = (shared_mem *) shmat (shmid, shmadd, 0)) == (shared_mem *) -1)
	{ 
		perror ("shmat");
		exit (0); 
	} 
	game->numSockets = 0;
	game->numReady = 0;
	game->gameStart = 0;
	setArray();
	/*Shared Memory now setup*/
	/*Setup Socket*/
   int sockfd, newsockfd, portno, clilen;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   int status, pid;
   
   /* First call to socket() function */
   sockfd = socket(AF_INET, SOCK_STREAM,DEFAULT_PROTOCOL );
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = PORTNUM;
   
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   
   /* Now bind the host address using bind() call.*/
   status =  bind(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)); 

   if (status < 0) {
      perror("ERROR on binding");
      exit(1);
   }
   
   /* Now Server starts listening clients wanting to connect. No more than 5 clients allowed */
   listen(sockfd,5);
   clilen = sizeof(cli_addr);
   
   while (1) {
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	  
      if (newsockfd < 0) {
         perror("ERROR on accept");
         exit(1);
      }
      
	  //*****Threading may have to be added here****//
      /* Create child process */
      pid = fork();
		
      if (pid < 0) {
         perror("ERROR on fork");
         exit(1);
      }
      
      if (pid == 0) {
         /* This is the client process */
		 game->numSockets += 1;
         doprocessing(newsockfd);
		 close(sockfd);
         exit(0);
      }
      else {
         close(newsockfd);
      }
		
   } /* end of while */
}


void doprocessing (int sock) {
	int status, i, j;
	char buffer[256], *arrStr;
	
	bzero(buffer,256);
	status= read(sock,buffer,255);
		
	if (status < 0) {
		perror("ERROR reading from socket");
		exit(1);
	}

	int ready = strncmp(buffer, "ready", 5);
	if(ready == 0) {
		printf("Client is ready\n");
		game->numReady += 1;
	}
	while (game->numReady < game->numSockets){}
	
	arrStr = getArrayStr();
	printf("start:\n%s\n\n", arrStr);
	status = write(sock, arrStr, 40);
	
	if (status < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}
}

void setArray() {
   int count = 0, i, j;
   
   for(i = 0; i < 4; i++) {
      for(j = 0; j < 4; j++) {
         game->gameArr[i][j] = 'a' + count;
         count++;
      }
   }
}

char* getArrayStr (){
	int i, j, a=0;
	char arrStr[40];
	
	for(i = 0; i < 4; i++) {
          for(j = 0; j < 4; j++) {
             arrStr[a] = game->gameArr[i][j];
	     a++;
	     if (j < 3){
	       arrStr[a] = '\t';
               a++;
	     }
          }
          if (i < 3){
	    arrStr[a]= '\n';
	    a++;
          }
       }
   return arrStr;
}
