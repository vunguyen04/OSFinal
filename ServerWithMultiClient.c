#include <string.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

#define PORTNUM  5001 /* the port number the server will listen to*/
#define DEFAULT_PROTOCOL 0  /*constant for default protocol*/
define ARR_SIZE 4

//Global Variable Declarations
char **arrA;

void doprocessing (int sock);
void setArray ();

int main( int argc, char *argv[] ) {
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
      
      /* Create child process */
      pid = fork();
		
      if (pid < 0) {
         perror("ERROR on fork");
         exit(1);
      }
      
      if (pid == 0) {
         /* This is the client process */
         close(sockfd);
         doprocessing(newsockfd);
         exit(0);
      }
      else {
         close(newsockfd);
      }
		
   } /* end of while */
}


void doprocessing (int sock) {
   int status;
   char buffer[256];
   bzero(buffer,256);
   status= read(sock,buffer,255);
   
   if (status < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
   
   //printf("Here is the message: %s\n",buffer);
   //status= write(sock,"I got your message",18);
   if(strcmp(buffer, "ready") == 0) {
      printf("Client is ready");

      for(int i = 0; i < ARR_SIZE; i++) {
         for(int j = 0; j < ARR_SIZE; j++) {
            printf("%c ", arrA[i][j]);
         } 
         printf("\n");
      }
   }

   if (status < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
	
}

void setArray() {
   int count = 0;

  //Creating Array
   arrA = (char **)malloc(ARR_SIZE * sizeof(char *));
  
   for(int i = 0; i < ARR_SIZE; i++) {
      arrA[i] = (char *)malloc(ARR_SIZE * sizeof(char));
   }
   
   for(int i = 0; i < ARR_SIZE; i++) {
      for(int j = 0; j < ARR_SIZE; j++) {
         arrA[i][j] = 'a' + count;
         count++;
      }
   }
   
}
