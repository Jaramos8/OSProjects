#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

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

int my_write_string(int fd, char *str) {
  size_t len, wrlen;
  ssize_t wlen;

  len = strlen(str);
  if (len == ((size_t) 0)) return 0;

  wlen = my_write(fd, str, len);
  if (wlen < ((ssize_t) 0)) return -1;
  wrlen = (size_t) wlen;
  if (wrlen != len) return -1;
  return 0;
}

int run_client(int fd, char *str) {
  char buffer[129];
  
  /* Write the string to the server */
  if (my_write_string(fd, str) < 0) {
    fprintf(stderr, "Could not write: %s\n", strerror(errno));
    return -1;
  }

  /* Read from the server */
  memset(buffer, '\0', sizeof(buffer));
  if (read(fd, buffer, sizeof(buffer) - ((size_t) 1)) < 0) {
    fprintf(stderr, "Could not read: %s\n", strerror(errno));
    return -1;
  }

  /* Display the string sent back from the server */
  printf("%s\n", buffer);

  /* Indicate success */
  return 0;
}

/* client server-address server-port string */
int main(int argc, char **argv) {
  char *addr_str, *port_str, *str;
  int gai_code;
  struct addrinfo hints;
  struct addrinfo *result, *curr;
  int found;
  int sockfd;
  
  /* Check if we got enough arguments */
  if (argc < 4) {
    fprintf(stderr, "Not enough arguments\n");
    return 1;
  }
  addr_str = argv[1];
  port_str = argv[2];
  str = argv[3];
  if ((addr_str[0] == '\0') ||
      (port_str[0] == '\0') ||
      (str[0] == '\0')) {
    fprintf(stderr, "Arguments must not be empty\n");
    return 1;
  }

  /* Get the server address info */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  hints.ai_flags = 0;
  gai_code = getaddrinfo(addr_str, port_str, &hints, &result);
  if (gai_code != 0) {
    fprintf(stderr, "Could not look up the server address: %s\n",
	    gai_strerror(gai_code));
    return 1;
  }
  /* The result is a linked list of results */
  for (found=0, curr=result; curr!=NULL; curr=curr->ai_next) {
    /* Per result, we need to create a socket
       and try to connect over that socket.

       We stop once both operations go through.
    */
    sockfd = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);
    if (sockfd >= 0) {
      /* We got a socket, we need to try to connect that socket. */
      if (connect(sockfd, curr->ai_addr, curr->ai_addrlen) >= 0) {
	found = 1;
	break;
      }
      if (close(sockfd) < 0) {
	fprintf(stderr, "Could not close a socket: %s\n", strerror(errno));
	freeaddrinfo(result);
	return 1;
      }
    }
  }
  /* Free the linked list of results, which resides in heap memory */
  freeaddrinfo(result);
  /* If we did not find anything, we need to fail */
  if (!found) {
    fprintf(stderr, "Could not connect to any server at the specified address and port\n");
    return 1;
  }

  /* We have a valid file descriptor in sockfd 

     So we work with the server.
     
  */
  if (run_client(sockfd, str) < 0) {
    if (close(sockfd) < 0) {
      fprintf(stderr, "Could not close a socket: %s\n", strerror(errno));
    }    
    return 1;
  }

  /* Everything fine, just close the socket */
  if (close(sockfd) < 0) {
    fprintf(stderr, "Could not close a socket: %s\n", strerror(errno));
  }    
  
  /* Indicate success */
  return 0;
}
