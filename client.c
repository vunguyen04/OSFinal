/* this program shows how to create sockets for a client.
it also shows how the client connects to a server socket.
and sends a message to it. the server must already be running
on a machine. The name of this machine must be entered in the function gethostbyname in the code below. The port number where the server is listening is specified in PORTNUM. This port number must also be specified in the server code.

 * main program */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

#define PORTNUM  1107 /* the port number that the server is listening to*/
#define DEFAULT_PROTOCOL 0  /*constant for default protocol*/
#define h_addr h_addr_list[0]

void main() {
   int  port;
   int  socketid;      /*will hold the id of the socket created*/
   int  status;        /* error status holder*/
   char buffer[256];   /* the message buffer*/
   struct sockaddr_in serv_addr;
   struct hostent *server;

//-------------------------------------Socket Initialization--------------------------------------------------
   /* this creates the socket*/
   socketid = socket (AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
   
   if (socketid < 0) {
      printf( "error in creating client socket\n"); 
      exit (-1);
   }

   printf("created client socket successfully\n");

   /* before connecting the socket we need to set up the right values in the different fields of the structure server_addr 
   you can check the definition of this structure on your own*/
   
   server = gethostbyname("osnode02"); 

   if (server == NULL) {
      printf(" error trying to identify the machine where the server is running\n");
      exit(0);
   }

   port = PORTNUM;
   
   /*This function is used to initialize the socket structures with null values. */
   bzero((char *) &serv_addr, sizeof(serv_addr));

   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr,
		(char *)&serv_addr.sin_addr.s_addr,
		server->h_length); 
   serv_addr.sin_port = htons(port);
   
   /* connecting the client socket to the server socket */
   status = connect(socketid,
		(struct sockaddr *) &serv_addr,
		sizeof(serv_addr));

   if (status < 0) {
      printf( "error in connecting client socket with server\n");
      exit(-1);
   }
//-----------------------------------------------------------------------------------------------------------

//-------------------------------------Player Readying Up----------------------------------------------------
   char message[256];
   printf("connected client socket to the server socket \n");
   printf("Type 'ready' to continue: ");
   scanf("%s", message);

   //sending ready message to server
   bzero(buffer, 256);
   status = write(socketid, message, 5);
   if (status < 0) {   
	   printf("error while sending client message to server\n");
   }

   //server is telling the client what player they are
   bzero(buffer, 256);
      status = read(socketid, buffer, 255);
      if (status < 0){
         perror("ERROR while reading message from server");
         exit(1);
      }
   printf("You are player %c", buffer[0]);
   
   //Server will be first printing the array
   bzero(buffer,256);
   status = read(socketid, buffer, 255);
   if (status < 0) {
	   perror("error while reading message from server");
	   exit(1);
   }
   
   printf("\nRecieved:\n%s\n",buffer);
//-----------------------------------------------------------------------------------------------------------

//-------------------------------------Game Execution--------------------------------------------------------
   while (1){
      printf("Pick up your letter (a to p) & x to exit: ");
      scanf("%s", message);
      status  = write(socketid, message, 1);
      int check = strncmp(message, "x", 1);
      if (check == 0){
         printf("User Exit\n");
         break;
      }
      bzero(buffer, 256);
      status = read(socketid, buffer, 255);
      if (status < 0){
         perror("ERROR while reading message from server");
         exit(1);
      }
      //check to see if the game is over
      if(buffer[0] == '1' || buffer[0] == '2'|| buffer[0] == '3' || buffer[0] == '4' || buffer[0] == '5'){
         printf("Player %s wins!\n", buffer);
         break;
      }
      //if the game is not over, continue the game
      else {
         printf("New:\n%s\n",buffer);
      }
   }
   /* this closes the socket*/
   close(socketid);  
//-----------------------------------------------------------------------------------------------------------
} 

