#include "../include/utils.h"
#include "../include/server.h"
#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <stdint.h>

#define BUFF_SIZE 1028

//TODO: Declare a global variable to hold the file descriptor for the server socket
int master_fd;
//TODO: Declare a global variable to hold the mutex lock for the server socket
pthread_mutex_t socket_mutex;
//TODO: Declare a gloabl socket address struct to hold the address of the server
struct sockaddr_in server_addr;

/*
################################################
##############Server Functions##################
################################################
*/

/**********************************************
 * init
   - port is the number of the port you want the server to be
     started on
   - initializes the connection acception/handling system
   - if init encounters any errors, it will call exit().
************************************************/
void init(int port) {
   //TODO: create an int to hold the socket file descriptor
   //TODO: create a sockaddr_in struct to hold the address of the server
  int sd;
  struct sockaddr_in server;
   /**********************************************
    * IMPORTANT!
    * ALL TODOS FOR THIS FUNCTION MUST BE COMPLETED FOR THE INTERIM SUBMISSION!!!!
    **********************************************/
   
   
   
   // TODO: Create a socket and save the file descriptor to sd (declared above)
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }
   // TODO: Change the socket options to be reusable using setsockopt(). 
  int opt = 1;
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("Set socket options failed");
    close(sd);
    exit(EXIT_FAILURE);
  }

   // TODO: Bind the socket to the provided port.
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);

  if (bind(sd, (struct sockaddr *)&server, sizeof(server)) < 0) {
    perror("Bind failed");
    close(sd);
    exit(EXIT_FAILURE);
  }
   // TODO: Mark the socket as a pasive socket. (ie: a socket that will be used to receive connections)

  if (listen(sd, SOMAXCONN) < 0) {
    perror("Listen failed");
    close(sd);
    exit(EXIT_FAILURE);
  }
   
   
   // We save the file descriptor to a global variable so that we can use it in accept_connection().
   // TODO: Save the file descriptor to the global variable master_fd
  master_fd = sd;
  printf("UTILS.O: Server Started on Port %d\n",port);
  fflush(stdout);

}


/**********************************************
 * accept_connection - takes no parameters
   - returns a file descriptor for further request processing.
   - if the return value is negative, the thread calling
     accept_connection must should ignore request.
***********************************************/
int accept_connection(void) {

   //TODO: create a sockaddr_in struct to hold the address of the new connection
   struct sockaddr_in addr;
   socklen_t addr_len = sizeof(addr);
   
   /**********************************************
    * IMPORTANT!
    * ALL TODOS FOR THIS FUNCTION MUST BE COMPLETED FOR THE INTERIM SUBMISSION!!!!
    **********************************************/
   
   
   
   // TODO: Aquire the mutex lock
   if(pthread_mutex_lock(&socket_mutex)!=0){
    perror("LOCK FAIL\n");
    return -1;
   }
   // TODO: Accept a new connection on the passive socket and save the fd to newsock
  int acceptFd = accept(master_fd,(struct sockaddr*)&addr,&addr_len);
  if(acceptFd < 0){
    perror("ACCEPT FAIL");
    if(pthread_mutex_unlock(&socket_mutex)!=0){
      perror("UNLOCK FAIL\n");
      return -1;
    }
    return -1;
  }
   // TODO: Release the mutex lock
  if(pthread_mutex_unlock(&socket_mutex)!=0){
    perror("UNLOCK FAIL\n");
    return -1;
   }

   // TODO: Return the file descriptor for the new client connection
   return acceptFd;

}


/**********************************************
 * send_file_to_client
   - socket is the file descriptor for the socket
   - buffer is the file data you want to send
   - size is the size of the file you want to send
   - returns 0 on success, -1 on failure 
************************************************/
int send_file_to_client(int socket, char * buffer, int size) 
{
    //TODO: create a packet_t to hold the packet data
	packet_t packet;
	packet.size = size;
	if(write(socket, &packet.size, sizeof(packet.size)) < 0){
		perror("Failed to send file size!");
		return -1;
	}

    //TODO: send the file size packet


    //TODO: send the file data
	int bytes_sent = 0;
    while (bytes_sent < size) {
        int chunk_size = (size - bytes_sent) > BUFF_SIZE ? BUFF_SIZE : (size - bytes_sent);
        memcpy(buffer, buffer + bytes_sent, chunk_size);

        if (write(socket, buffer, chunk_size) < 0) {
            perror("Failed to send file data");
            return -1;
        }
        bytes_sent += chunk_size;
    }
	printf("send_file_to_client is success!");
    //TODO: return 0 on success, -1 on failure
	return 0;
}


/**********************************************
 * get_request_server
   - fd is the file descriptor for the socket
   - filelength is a pointer to a size_t variable that will be set to the length of the file
   - returns a pointer to the file data
************************************************/
char * get_request_server(int fd, size_t *filelength)
{
    //TODO: create a packet_t to hold the packet data
	packet_t packet;
	
    //TODO: receive the response packet
	if (read(fd, &packet.size, sizeof(packet.size)) <= 0) {
        perror("Failed to receive file size");
        return NULL;
    }
	char *buffer = malloc(*filelength);
    if (!buffer) {
        perror("Failed to allocate memory for file data");
        return NULL;
    }

    //TODO: get the size of the image from the packet

    //TODO: recieve the file data and save into a buffer variable.
	int bytes_received = 0;
    while (bytes_received < *filelength) {
        int chunk_size = (*filelength - bytes_received) > BUFF_SIZE ? BUFF_SIZE : (*filelength - bytes_received);
        int received = read(fd, buffer + bytes_received, chunk_size);
        if (received <= 0) {
            perror("Failed to receive file data");
            free(buffer);
            return NULL;
        }
        bytes_received += received;
    }

    //TODO: return the buffer
	printf("get_request_server is much success!");
	return buffer;
}


/*
################################################
##############Client Functions##################
################################################
*/

/**********************************************
 * setup_connection
   - port is the number of the port you want the client to connect to
   - initializes the connection to the server
   - if setup_connection encounters any errors, it will call exit().
************************************************/
int setup_connection(int port)
{
    //TODO: create a sockaddr_in struct to hold the address of the server   
    struct sockaddr_in serveradd;
    //TODO: create a socket and save the file descriptor to sockfd
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
      perror("ERROR opening socket");
      exit (-1);
    }
    //TODO: assign IP, PORT to the sockaddr_in struct
    serveradd.sin_family = AF_INET;
    serveradd.sin_port = htons((unsigned short)port);
    //TODO: connect to the server
    if (connect(sockfd, (struct sockaddr *)&serveradd, sizeof(serveradd)) < 0) {
      perror("ERROR connecting");
      exit (-1);
    }
    //TODO: return the file descriptor for the socket
    return sockfd;
}


/**********************************************
 * send_file_to_server
   - socket is the file descriptor for the socket
   - file is the file pointer to the file you want to send
   - size is the size of the file you want to send
   - returns 0 on success, -1 on failure
************************************************/
int send_file_to_server(int socket, FILE *file, int size) 
{
    //TODO: send the file size packet

    //int n = write(socket, , size);
    //if(n<0){
      //perror("ERROR writing to socket");
    //}

    //TODO: send the file data

   

    // TODO: return 0 on success, -1 on failure
   
}

/**********************************************
 * receive_file_from_server
   - socket is the file descriptor for the socket
   - filename is the name of the file you want to save
   - returns 0 on success, -1 on failure
************************************************/
int receive_file_from_server(int socket, const char *filename) 
{
    //TODO: create a buffer to hold the file data
    char buffer[BUFFER_SIZE];
    packet_t size_packet;
    size_t bytes_received = 0;
    size_t total_bytes = 0;


    //TODO: open the file for writing binary data
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Failed to open file");
        return -1;
    }

    
   //TODO: create a packet_t to hold the packet data
   //TODO: receive the response packet
    if (recv(socket, &size_packet, sizeof(packet_t), 0) <= 0) {
      perror("Failed to receive size packet");
      fclose(file);
      return -1;

    }

   //TODO: get the size of the image from the packet
   total_bytes = size_packet.size;

   //TODO: recieve the file data and write it to the file
    while (bytes_received < total_bytes) {
        size_t remaining = total_bytes - bytes_received;
        size_t to_read = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
        
        ssize_t chunk = recv(socket, buffer, to_read, 0);
        
        if (chunk <= 0) {
            perror("Failed to receive file data");
            fclose(file);
            return -1;
        }

        if (fwrite(buffer, 1, chunk, file) != chunk) {
            perror("Failed to write to file");
            fclose(file);
            return -1;
        }

        bytes_received += chunk;
    }

    fclose(file);

    //TODO: return 0 on success, -1 on failure
    return 0;


}



