#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT ((short int) 9999)

void to_upper_case(char *str) {
  char *curr;

  for (curr=str; *curr!='\0'; curr++) {
    if (('a' <= *curr) && (*curr <= 'z')) {
      *curr = (char) (((int) *curr) - ((int) 'a') + ((int) 'A'));
    }
  }
}

ssize_t my_write(int fd, const void *buf, size_t count) {
  size_t to_be_written, already_written, written_this_time;
  ssize_t tmp;

  if (count == ((size_t) 0)) return (ssize_t) 0;
  
  for (to_be_written=count, already_written=(size_t) 0;
       to_be_written > (size_t) 0;
       to_be_written -= written_this_time, already_written += written_this_time) {
    tmp = write(fd, buf+already_written, to_be_written);
    if (tmp < ((ssize_t) 0)) return tmp;
    written_this_time = (size_t) tmp;
  }
  return (ssize_t) already_written;
}

int run_server(int fd) {
  char buffer[129];
  size_t len;

  /* Read a string from the client */
  memset(buffer, '\0', sizeof(buffer));
  if (read(fd, buffer, sizeof(buffer) - ((size_t) 1)) < 0) {
    fprintf(stderr, "Could not read: %s\n", strerror(errno));
    return -1;
  }

  /* Convert the string to upper case */
  to_upper_case(buffer);

  /* Write the string back to the client */
  len = strlen(buffer);
  if (my_write(fd, buffer, len) != ((ssize_t) len)) {
    fprintf(stderr, "Could not write: %s\n", strerror(errno));
    return -1;
  }

  /* Indicate success */
  return 0;
}

int main(int argc, char **argv) {
  int sockfd, connfd;
  struct sockaddr_in serveraddr, clientaddr;
  socklen_t clientaddrlen;

  /* Get a socket for Internet Protocol version 4

     Reliable protocol, like a Circuit-Switched connection => stream

  */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    fprintf(stderr, "Could not create a socket: %s\n", strerror(errno));
    return 1;
  }

  /* Bind the socket */
  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(PORT);
  if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
    fprintf(stderr, "Could not bind a socket: %s\n", strerror(errno));
    if (close(sockfd) < 0) {
      fprintf(stderr, "Could not close a socket: %s\n", strerror(errno));
    }    
    return 1;
  }

  /* Await the connection of a client by listening on the port */
  if (listen(sockfd, 5) < 0) {
    fprintf(stderr, "Could not listen on a socket: %s\n", strerror(errno));
    if (close(sockfd) < 0) {
      fprintf(stderr, "Could not close a socket: %s\n", strerror(errno));
    }    
    return 1;
  }

  /* Accept the connection from the client */
  clientaddrlen = (socklen_t) sizeof(clientaddr);
  connfd = accept(sockfd, (struct sockaddr *) &clientaddr, &clientaddrlen);
  if (connfd < 0) {
    fprintf(stderr, "Could not accept on a socket: %s\n", strerror(errno));
    if (close(sockfd) < 0) {
      fprintf(stderr, "Could not close a socket: %s\n", strerror(errno));
    }    
    return 1;
  }
  if (((size_t) clientaddrlen) != sizeof(clientaddr)) {
    fprintf(stderr, "Could not get full client address\n");
    if (close(connfd) < 0) {
      fprintf(stderr, "Could not close a connection: %s\n", strerror(errno));
    }
    if (close(sockfd) < 0) {
      fprintf(stderr, "Could not close a socket: %s\n", strerror(errno));
    }    
    return 1;
  }

  /* Work with the client */
  if (run_server(connfd) < 0) {
    if (close(connfd) < 0) {
      fprintf(stderr, "Could not close a connection: %s\n", strerror(errno));
    }
    if (close(sockfd) < 0) {
      fprintf(stderr, "Could not close a socket: %s\n", strerror(errno));
    }    
    return 1;
  }

  /* Close the connection */
  if (close(connfd) < 0) {
    fprintf(stderr, "Could not close a connection: %s\n", strerror(errno));
    if (close(sockfd) < 0) {
      fprintf(stderr, "Could not close a socket: %s\n", strerror(errno));
    }    
    return 1;    
  }
 
  /* Close the socket */
  if (close(sockfd) < 0) {
    fprintf(stderr, "Could not close a socket: %s\n", strerror(errno));
    return 1;
  }

  /* Signal success */
  return 0;
}
