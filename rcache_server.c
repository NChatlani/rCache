#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#define MAX_CONNECTION_QUEUE_LEN 1024
#define PORT 10237

int max = 20;
int *buffer;

int use = 0;
int fill = 0;
int consumers = 10;

sem_t empty;
sem_t full;
sem_t mutex;

/* Hashtable structure definitions */
typedef struct _hashnode_t {
  char *key;
  char *value;
  struct _hashnode_t *next;
} hashnode_t;

typedef struct _hash_t {
  hashnode_t **buckets;
  int size;
} hashtable_t;

typedef struct arg_struct {
  struct sockaddr_in arg1;
  int arg2;
} args;


/*
* Read a null-terminated string from a socket
*
* Inputs:
*     int fd: the socket representing the connection to the client
*     char *buffer: the buffer that will hold the string
*
*/
void readString(int fd, char *buffer) {
    int i = 0;
    while (1) {
      read(fd, &buffer[i], sizeof(char));

      if (buffer[i] == '\0')
        break;
      else
        i++;
    }
}

void writeString(char *s, int socket_fd) {
  int rc;
  if(s == NULL){
    char z = '\0';
    rc = write(socket_fd, &z, sizeof(char));
  } 
  else {
    rc = write(socket_fd, s, strlen(s) + 1); // +1 for the terminating NULL byte
  }
  if (rc < 0) {
    perror("writeString");
    exit(1);
  }
}


/*
* Hash a string to an integer
*
* Input
*    char *s: the string to hash
* Output
*    The integer hash code
*/
unsigned long int hash(char *s) {
  unsigned long int h = 0;

  int i;
  for (i = 0; i < strlen(s); i++) {
    h += (unsigned long int) s[i];
  }

  return h;
}


/*
* Create a new hashtable_t
*
* Input
*     tableSize: the number of buckets in the new table
* Output
*    A pointer to the new table
*/
hashtable_t *hashtableInit(int tableSize) {

  hashtable_t *table = NULL;
  
  if (tableSize < 1) return NULL;
  
  if((table = malloc(sizeof(hashtable_t))) == NULL) return NULL;
  
  if((table->buckets = malloc(sizeof(hashtable_t) * tableSize)) == NULL) return NULL;
  
  int i;
  for(i = 0; i < tableSize; i++){
    table->buckets[i] = NULL;
  }
  
  table->size = tableSize;
  
  return table;
}


/*
* Insert a key-value pair into a table
*
* Inputs
*    hashtable *h: the hashtable performing the insertion
*    int connection_fd: the socket connection to the client
* Output
*    Nothing; a response indicating success (0) or failure (-1) is sent
*    back to the client
*/
void hashtableInsert(hashtable_t *h, int connection_fd) {
  
  char key[128];
  char value[1024];
  
  readString(connection_fd, key);
  readString(connection_fd, value);
  
  // Hash the input and determine the bucket that will hold the new pair
  int hashcode = hash(key);
  int b = hashcode % h->size;

  // Create a new hashnode_t
  
  hashnode_t *node = malloc(sizeof(hashnode_t));
  node->key = strdup(key);
  node->value = strdup(value);
  node->next = malloc(sizeof(hashnode_t));

  // Insert into the bucket's list
  
  node->next = h->buckets[b];
  h->buckets[b] = node;
  
  int r  = 0;
  write(connection_fd, &r, sizeof(int));

}


/*
* Lookup the value for a given key and send it back to the client
*
* Inputs
*    hashtable *h: the hashtable performing the insertion
*    int connection_fd: the socket representing the connection to the client
* Output
*    Nothing; the value string is sent to the client over the socket
     returns a NULL string if the key is not in the table
*/
void hashtableLookup(hashtable_t *h, int connection_fd) {
  
  char key[128];
  
  readString(connection_fd, key);

  int hashcode = hash(key);
  int b = hashcode % h->size;
  
  hashnode_t *current = h->buckets[b];
  
  while(current != NULL && strcmp(key, current->key) != 0){
    current = current->next;
  }
  
  if(current == NULL){
    writeString(NULL, connection_fd);
  }
  else{
    writeString(current->value, connection_fd);
  }
  
}


/*
* Remove the key-value pair with the given key from the table. The value string
* is returned to the client, or a NULL string is returned if the key is not
* in the table.
*
* Inputs
*    hashtable *h: the hashtable performing the insertion
*    int connection_fd: the socket representing the connection to the client
* Output
*    Nothing; the value string is sent to the client over the socket
*/
void hashtableRemove(hashtable_t *h, int connection_fd) {
  
  char key[128];
  
  readString(connection_fd, key);

  int hashcode = hash(key);
  int b = hashcode % h->size;
  
  hashnode_t *current = h->buckets[b];
  hashnode_t *prev = NULL;
  
  if(current == NULL){
    writeString(NULL, connection_fd);
    return;
  }
  
  //IF HEAD IS THE ONE TO REMOVE
  if(strcmp(key, current->key) == 0){
    char *ret = current-> value;
    h->buckets[b] = current->next;
    writeString(ret, connection_fd);
    free (current);
    return;
  }
  
  else{
    while(current != NULL && strcmp(key, current->key) != 0){
      prev = current;
      current = current->next;
    }
  
    if(current == NULL){
      writeString(NULL, connection_fd);
    }
    else{
      char * ret = current->value;
      prev->next = current->next;
      current->next = NULL;
      writeString(ret, connection_fd);
      free (current);
    }
    
  }
  
}


/*
* Print the hastable to the server's console
*
* Input:
*     hastable_t *h: pointer to the table
*/
void hashtablePrint(hashtable_t *h) {
  int i;
  for (i = 0; i < h->size; i++) {
    hashnode_t *node = h->buckets[i];
    printf("Contents of bucket %d:\n", i);
    while (node != NULL) {
      printf("\t<%s, %s>\n", node->key, node->value);
      node = node->next;
    }
  }
  printf("\n");
}

void do_fill(int value) {
    buffer[fill] = value;
    fill++;
    if (fill == max)
	fill = 0;
}


int do_get() {
    int tmp = buffer[use];
    use++;
    if (use == max)
	use = 0;
    return tmp;
}

void * producer(void *arg){
  
  int i;
  struct arg_struct *readArg = (struct arg_struct*) arg;
  
  while(1){
      socklen_t sa_len = sizeof(readArg->arg1);
      int connection_fd = accept(readArg->arg2, (struct sockaddr*) &readArg->arg1, &sa_len);
      if (connection_fd < 0) {
        perror("accept");
        exit(1);
      }
    
    sem_wait(&empty);
    
    sem_wait(&mutex);
    
    do_fill(connection_fd);
    
    sem_post(&mutex);
    
    sem_post(&full);
    
  }
   for (i = 0; i < consumers; i++) {
	   sem_wait(&empty);
	   sem_wait(&mutex);
	   do_fill(-1);
	   sem_post(&mutex);
	   sem_post(&full);
    }

    return NULL;
  
}

void * consumer(void *arg){
  
  hashtable_t *table = (hashtable_t *) arg;
  int tmp = 0;
  
  while (tmp != -1){
    sem_wait(&full);
    
    sem_wait(&mutex);
    
    tmp = do_get();
    
    sem_post(&mutex);
    
    sem_post(&empty);
    
    char cmd;

    int rc = read(tmp, &cmd, sizeof(char));
    
    // Decode the command and call the appropriate hash table function
    if(cmd == 'I'){
      hashtableInsert(table, tmp);
    }
    else if(cmd == 'L'){
      hashtableLookup(table, tmp);
    }
    else if(cmd == 'R'){
      hashtableRemove(table, tmp);
    }
    else if(cmd == 'P'){
      hashtablePrint(table);
    }
    else{
      printf("Not a valid function\n");
      continue;
    }

    // Close the connection
    rc = close(tmp);
    if (rc < 0){
      perror("close");
      exit(1);
    }
  }
  
  return NULL;
}

/*
* Main: create the server connection and make it listen for client messages
*/
int main(int argc, char** argv) {

  // Create the socket
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket");
    return(-1);
  }

  // Initialize the sockaddr structure
  int port = PORT;

  struct sockaddr_in sa;
  bzero((char *) &sa, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  sa.sin_port = htons((unsigned short) port);

  // Bind the socket descriptor to the address
  int rc = bind(server_fd, (struct sockaddr*) &sa, sizeof(sa));
  if (rc < 0) {
    perror("bind");
    return(-1);
  }

  // Make the socket listen for incoming requests
  int max_queue_length = MAX_CONNECTION_QUEUE_LEN;

  rc = listen(server_fd, max_queue_length);
  if (rc < 0) {
    perror("listen");
    return(-1);
  }

  // Initialize a new hash table
  hashtable_t * table = hashtableInit(25);
  
  buffer = (int *) malloc(max * sizeof(int));
  int i;
  for (i = 0; i < max; i++) {
	 buffer[i] = 0;
  }

  // Initialize semaphores
  sem_init(&empty, 0, max); // max are empty 
  sem_init(&full, 0,  0);    // 0 are full
  sem_init(&mutex, 0, 1);   // mutex
  
  struct arg_struct readArgs;
  readArgs.arg1 = sa;
  readArgs.arg2 = server_fd;
  
  pthread_t pid, cid[consumers];
  pthread_create(&pid, NULL, producer, &readArgs); 
  for (i = 0; i < consumers; i++) {
	 pthread_create(&cid[i], NULL, consumer, table); 
  }
  
  pthread_join(pid, NULL); 
  for (i = 0; i < consumers; i++) {
	 pthread_join(cid[i], NULL); 
  }

  return 0;
}
