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

#define PORTNUM  1107 /* the port number the server will listen to*/
#define DEFAULT_PROTOCOL 0  /*constant for default protocol*/
#define ARR_SIZE 4

#define SEMKEY ((key_t) 400L)
#define SHMKEY ((key_t) 7890) 
#define NSEMS 1  /*defines the number of semaphores to be created*/

typedef struct{
	char gameArr[4][4];
	int numSockets;
	int numReady;
	int gameStart[5];
   int gameScores[5];
   int endGame;
} shared_mem;

//-------------------------------------Global Variable/Function Declarations---------------------------------
shared_mem *game;

int pointArr[4][4];
void doprocessing (int sock, int playerNum);
void setArray ();
char* getArrayStr ();
int changeArray(char Selection, int socketNumber, int* available);
char findWinner();
//-----------------------------------------------------------------------------------------------------------

int main( int argc, char *argv[] ) {
//-------------------------------------Shared Memory Initialization------------------------------------------
   key_t key = 123; 
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
//-----------------------------------------------------------------------------------------------------------

	game->numSockets = 0;
	game->numReady = 0;
	//game->gameStart = 0;
   game->endGame = 16;

	setArray();

//-------------------------------------Socket Initialization--------------------------------------------------
   int sockfd, newsockfd, portno, clilen;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   int status, pid;
   
   sockfd = socket(AF_INET, SOCK_STREAM,DEFAULT_PROTOCOL ); //First call to socket() function
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr)); //Initialize socket structure 
   portno = PORTNUM;
   
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   
   status =  bind(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)); //Now bind the host address using bind() call

   if (status < 0) {
      perror("ERROR on binding");
      exit(1);
   }

   listen(sockfd,5);
   clilen = sizeof(cli_addr); //Now Server starts listening clients wanting to connect. No more than 5 clients allowed 
//-----------------------------------------------------------------------------------------------------------

//-------------------------------------Socket Listening/Game Initialization----------------------------------
   while (1) {
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	  
      if (newsockfd < 0) {
         perror("ERROR on accept");
         exit(1);
      }
      
      pid = fork(); //Create child process, threading may have to be added here
		
      if (pid < 0) {
         perror("ERROR on fork");
         exit(1);
      }
      
      if (pid == 0) { //This is the child process
		   game->numSockets += 1;
         game->gameStart[game->numSockets-1] = 1;
         doprocessing(newsockfd, game->numSockets);
         game->numSockets -= 1;
         close(sockfd);
         exit(0);
      }
      else {
         close(newsockfd);
      }
//-----------------------------------------------------------------------------------------------------------
   } 
}


void doprocessing (int sock, int playerNum) {
	int status, i, j, value;
   int socketNumber = (game->numSockets - 1); //use socket count as score index
	char buffer[256], *arrStr;

   printf("Player %d has joined\n", playerNum);

//-------------------------------------Player Ready-Up-------------------------------------------------------
   while(1) {
      while(!readyUp()){}

	   status= read(sock,buffer,255);
	   if (status < 0) {
		   perror("ERROR reading from socket");
		   exit(1);
	   }

      int ready = strncmp(buffer, "ready", 5);
      if(ready == 0) {
         printf("Player %d is ready\n", playerNum);
         buffer[0] = playerNum + '0';
         status = write(sock, buffer, 1);
         if (status < 0) {
            perror("ERROR writing to socket");
            exit(1);
         }
         game->numReady += 1;
      }

      while (game->numReady < game->numSockets){}
      setArray();
      game->endGame = 16;
      bzero(buffer,256);
      
      arrStr = getArrayStr();
      printf("start:\n%s\n\n", arrStr);
      status = write(sock, arrStr, 40);
//-----------------------------------------------------------------------------------------------------------

//-------------------------------------Game Execution--------------------------------------------------------

      while (game->endGame > 0){
         bzero(buffer,256); //empty buffer and read input from user
         status = read(sock, buffer, 255);
         if (status < 0){
            perror("ERROR writing to socket");
            exit(1);
         }
         
         int check = strncmp(buffer, "x", 1); //check for exit input x
         if (check == 0){
            printf("User Quit\n");
            break;
         }
   
         char Selection = buffer[0];
         int available;
         value = changeArray(Selection, socketNumber, &available);

         if (available == 1){//check if letter is available
           printf("Input from player %d: %s %d\n", playerNum, buffer, value);
           printf("Player %d's score is now: %d\n", playerNum, game->gameScores[socketNumber]);

           if(game->endGame == 0){ //checking the end game condition and returning who won
              buffer[0] = findWinner();
              printf("Player %c wins!\n", buffer[0]);
              break;
           }
           else { //if endgame is not met, continue the game
              arrStr = getArrayStr();
              char arrStr1[256] = "Keep Going\n";
              strcat(arrStr1, arrStr);
              status = write(sock,arrStr1, 51);
              if (status < 0) {
                perror("ERROR writing to socket");
                exit(1);              
              }
           }
         }
         else{//if letter not available
           arrStr = getArrayStr();
           char arrStr1[256] = "Letter Has Been Chosen! Retry!\n";
           strcat(arrStr1, arrStr);
           status = write(sock, arrStr1, 71);
           if (status < 0){
             perror("ERROR writing to socket");
             exit(1);
           }
         }      
      }
      status = write(sock, buffer, 1);
      if (status < 0) {
		   perror("ERROR writing to socket");
		   exit(1);
	   }
      game->gameStart[playerNum -1] = 1;
      game->numReady -= 1;
   }
//-----------------------------------------------------------------------------------------------------------
}


int changeArray(char Selection, int socketNumber, int* available){
  int i, j, value;
  
  for(i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      if (game->gameArr[i][j] == Selection){
         int value = pointArr[i][j];
         game->gameScores[socketNumber] += value;
         if(value > 0) {
            game->gameArr[i][j] = '+';
         }
         else {
            game->gameArr[i][j] = '-';
         }
         game->endGame--;
         *available = 1;
         return value;
      }
    }
  }
  *available = 0;
  return 99;
}    


void setArray() {
   int count = 0, i, j;

   for(i = 0; i < 5; i++){
      game->gameScores[i] = 0;
      game->gameStart[i] = 0;
   }
   
   for(i = 0; i < 4; i++) {
      for(j = 0; j < 4; j++) {
         game->gameArr[i][j] = 'a' + count;
         pointArr[i][j]= (rand() % 20) - 10;
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


char findWinner() {
   int i, max, index;
   char ret;
   max = game->gameScores[0];
   index = 0;

   for(i = 1; i < game->numSockets; i++){
      if(game->gameScores[i] > max) {
         index = i;
         max = game->gameScores[i];
      }
   }
   switch(index){
      case 0: ret = '1';
         break;
      case 1: ret = '2';
         break;
      case 2: ret = '3';
         break;
      case 3: ret = '4';
         break;
      case 4: ret = '5';
         break;
   }
   return ret;
}

int readyUp() {
   int i, flag = 1;
   for(i = 0; i < game->numSockets; i++) {
      if(game->gameStart[i] == 0) {
         return 0;
      }
   }
   return 1;
}
