#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

char *ADDRESS = "localhost";
char * PORT = "10237";

/*
* Create a new socket connection to the server
*
* Returns:
*   the int representing the new socket connection
*/
int createClientSocket() {

  // Initialize the address structure
  struct addrinfo *servinfo;
  struct addrinfo hints;

  // hints sets up the parameters for the connection
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int rc = getaddrinfo(ADDRESS, PORT, &hints, &servinfo);
  if (rc != 0) {
    perror("getaddrinfo");
    exit(1);
  }

  // Create the socket
  int client_fd = socket(servinfo->ai_family, servinfo->ai_socktype, 0);
  if (client_fd < 0) {
    perror("socket");
    return(-1);
  }

  // Connect the socket
  rc = connect(client_fd, servinfo->ai_addr, servinfo->ai_addrlen);
  if (rc < 0) {
    perror("connect");
    exit(1);
  }

  return client_fd;
}

/*
* Helper functions to write char and strings to a socket connection
*/
void writeChar(char c, int socket_fd) {
  int rc = write(socket_fd, &c, sizeof(char));
  if (rc < 0) {
    perror("writeChar");
    exit(1);
  }
  return;
}

void writeString(char *s, int socket_fd) {
  int rc = write(socket_fd, s, strlen(s) + 1); // +1 for the terminating NULL byte
  if (rc < 0) {
    perror("writeString");
    exit(1);
  }
}

void readString(int fd, char *buffer) {
    int i = 0;
    while (1) {
      int rc = read(fd, &buffer[i], sizeof(char));
      if(rc != sizeof(char)){
        perror("read from server");
        exit(1);
      }

      if (buffer[i] == '\0')
        break;
      else
        i++;
    }
}


/*
* Send the server a command to insert a <key, value> pair.
*
* Inputs:
*   char *key
*   char *value
*/
void rInsert(char *key, char *value) {
  char cmd = 'I';
  int clientSocket = createClientSocket();
  writeChar(cmd, clientSocket);
  writeString(key, clientSocket);
  writeString(value, clientSocket);

  int response;
  int rc = read(clientSocket, &response, sizeof(int));
  if (rc != sizeof(int)) {
    perror("read from server");
    exit(1);
  } else if (response != 0) {
    fprintf(stderr, "Insert operation failed. Exiting...\n");
    exit(1);
  }

  close(clientSocket);
  return;
}

/*
* Send a lookup command to the server. The server returns the associated value
* or NULL if no matching key can be found.
*
* Inputs:
*   char *key: the key to use for the lookup
*   char *value: pointer to a string that will receive the returned value
*/
void rLookup(char *key, char *value) {

  char cmd = 'L';
  int clientSocket = createClientSocket();
  writeChar(cmd, clientSocket);
  writeString(key, clientSocket);
  
  
  readString(clientSocket, value);

  close(clientSocket);
  return;
}

/*
* Send a remove command to the server. The serve returns the associated value
* or NULL if no matching value can be found.
*
* Inputs:
*   char *key: the key to use for the remove operation
*   char *value: pointer to a string that will receive the returned value
*/
void rRemove(char *key, char *value) {
  
  char cmd = 'R';
  int clientSocket = createClientSocket();
  writeChar(cmd, clientSocket);
  writeString(key, clientSocket);
  
  readString(clientSocket, value);
  
  close(clientSocket);
  return;
}

/*
* Send a print command to the server. The server will dump the contents of the
* hash table to its local console.
*/
void rPrint() {

  char cmd = 'P';
  int clientSocket = createClientSocket();
  
  writeChar(cmd, clientSocket);
  
  close(clientSocket);
  return;
}
