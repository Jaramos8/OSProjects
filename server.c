#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

/* Creates a new array of pointers to characters with argc+1 entries.

   Fills the first argc elements of that new array with copies of the
   strings in the argv array. Sets the last element of the new array
   to NULL. "Returns" the new array through the pointer
   new_argv. Signals success by returning zero.

   Does nothing if argc is negative, if an integer overflow occurs or
   if no memory can be allocated. Returns -1 in this case.

*/
int create_new_argv(char ***new_argv, int argc, char **argv) {
  size_t s;
  int t;
  char **temp_new_argv;
  char *str;
  size_t i;
  size_t l;
  size_t k;

  /* Check number of arguments */
  if (argc < 0) return -1;

  /* Check if argc fits on a size_t */
  s = (size_t) argc;
  t = (int) s;
  if (t != argc) return -1;

  /* Compute argc + 1 and check that no overflow occurs */
  s++;
  if (s < ((size_t) 1)) return -1;

  /* Allocate main array and check if memory allocation worked */
  temp_new_argv = (char **) calloc(s, sizeof(char *));
  if (temp_new_argv == NULL) return -1;

  /* Go over first argc elements */
  for (i=((size_t) 0);i<(s-((size_t) 1));i++) {
    /* Get length of string to be copied */
    l = strlen(argv[i]);

    /* Add one to string length to account for terminator.
       Check that no integer overflow occurs 
    */
    l++;
    if (l < ((size_t) 1)) {
      /* Integer overflow occurred. Deallocate everything
	 allocated so far.
      */
      for (k=((size_t) 0);k<i;k++) {
	free(temp_new_argv[k]);
      }
      free(temp_new_argv);
      return -1;
    }

    /* Allocate memory for a copy of the string argv[i].
       Check that memory allocation worked.
    */
    str = (char *) calloc(l, sizeof(char));
    if (str == NULL) {
      /* Memory allocation did not work. Deallocate everything
	 allocated so far.
      */      
      for (k=((size_t) 0);k<i;k++) {
	free(temp_new_argv[k]);
      }
      free(temp_new_argv);
      return -1;
    }
    /* Copy the string argv[i] into the copy */
    strncpy(str, argv[i], l);

    /* Put copy of string into main array */
    temp_new_argv[i] = str;
  }

  /* Set last element of new array to NULL */
  temp_new_argv[s] = NULL;

  /* "Return" the new array through the pointer */
  *new_argv = temp_new_argv;

  /* Indicate success */
  return 0;
}

/* Deallocates an array of allocated pointers to characters.

   Does nothing for NULL.

   Stops at the first string pointer that is NULL.

*/
void free_new_argv(char **argv) {
  char **curr;

  if (argv == NULL) return;
  for (curr=argv; *curr!=NULL; curr++) free(*curr);
  free(argv);
}

/* Runs the code for the child process:

   (i)    The child copies the arguments in argv.

   (ii)   The child copies the file descriptor connfd to have two 
          copies of that file descriptor, using dup().

   (iii)  Replaces standard input by the one file descriptor copy and
          standard output by the other file descriptor copy.

   (iv)   Replaces the executable by the one indicated by argv[0].

   Indicates failure by returning a negative value. Indicates success
   by returning zero or by simply never returning.

   In case of failure, deallocates all intermediately allocated memory
   and closes all copies of file descriptors created.

   In case of success or failure, does not close the connfd file
   descriptor.

*/
int run_child(int connfd, int argc, char **argv) {
  int in, out;
  char **new_argv;

  /* Copy arguments to fit format for execvp */
  if (/* TODO */) {
    return -1;
  }

  /* Duplicate file descriptor to have one for input and one for output */
  in = /* TODO */;
  if (in < 0) {
    fprintf(stderr, "Cannot duplicate a file descriptor: %s\n", strerror(errno));
    free_new_argv(new_argv);
    return -1;
  }
  
  /* Take original file descriptor as-is for the output file descriptor */
  out = connfd;

  /* Make input file descriptor have number of standard-in */
  if (/* TODO */) {
    fprintf(stderr, "Cannot execute make a file descriptor become standard input: %s\n", strerror(errno));
    if (close(in) < 0) {
      fprintf(stderr, "Cannot close a file descriptor: %s\n", strerror(errno));
    }
    free_new_argv(new_argv);
    return -1;
  }

  /* Make output file descriptor have number of standard-out */
  if (/* TODO */) {
    fprintf(stderr, "Cannot execute make a file descriptor become standard output: %s\n", strerror(errno));
    free_new_argv(new_argv);
    return -1;
  }
  
  /* Replace executable by the one given in arguments: execvp */
  if (/* TODO */) {
    fprintf(stderr, "Cannot replace executable: %s\n", strerror(errno));
    free_new_argv(new_argv);
    return -1;
  }

  /* Unreachable */
  free_new_argv(new_argv);
  return -1;
}

/* Runs the server on the input and output file descriptor connfd.

   The server starts by forking a child process. The child process
   inherits the file descriptor connfd and the memory for argc and argv.
   The child process immediately executes the run_child() function 
   defined above. 

   The parent server process then waits for the child to terminate and
   gets the child's exit status value.

   If anything goes wrong in the child, this function ensures
   that the connfd file descriptor gets closed in the child process.

   If anything goes wrong in this parent process, the server does not
   close the connfd file descriptor.

   Returns zero if everything went well and the child had an exit
   status of zero. Returns a negative value otherwise.

*/

int run_server(int connfd, int argc, char **argv) {
  pid_t kid;
  int wstatus, status;
  
  /* Fork off a child */
  kid = /* TODO */;

  /* Could not fork */
  if (/* TODO */) {
    fprintf(stderr, "Cannot fork off a child process: %s\n", strerror(errno));
    return -1;
  }

  /* In the child */
  if (/* TODO */) {
    if (run_child(connfd, argc, argv) < 0) {
      if (close(connfd) < 0) {
	fprintf(stderr, "Cannot close a socket: %s\n", strerror(errno));
      }
      exit(1);
    }
    if (close(connfd) < 0) {
      fprintf(stderr, "Cannot close a socket: %s\n", strerror(errno));
    }
    exit(1);
    /* Unreachable */
    return -1;
  }

  /* Parent process: wait for child and get exit status: waitpid */
  if (/* TODO */) {
    fprintf(stderr, "Cannot wait: %s\n", strerror(errno));
    return -1;
  }
  status = (int) ((char) WEXITSTATUS(wstatus));

  /* If child status is 0, return 0, otherwise return -1. */
  if (status == 0) return 0;
  return -1;
}

/* Tries to convert the string str to an unsigned short int.

   On success, assigns the converted integer to *n and returns zero.

   On failure, does not touch n and returns a negative value.

   Failure cases:

   * The string pointer str is NULL.
   * The string is empty (i.e. it starts with a terminator)
   * The conversion with strtoll fails.
   * strtoll yields a negative value.
   * strtoll yields a value that is not representable on 
     an unsigned short int.

*/
int try_convert_unsigned_short(unsigned short int *n, const char *str) {
  char *end;
  long long int nn;
  unsigned short int t;
  long long int tt;

  if (str == NULL) return -1;
  if (*str == '\0') return -1;
  nn = strtoll(str, &end, 0);
  if (*end != '\0') return -1;
  if (nn < ((long long int) 0)) return -1;
  t = (unsigned short int) nn;
  tt = (long long int) t;
  if (tt != nn) return -1;
  *n = t;
  return 0;
}


/* Server program: runs a TCP/IP server on a port and waits for a
   client to connect.  When a client connects, executes the executable
   indicated in the second argument of the server program, with the
   arguments following the second argument of the server program.

   Synopsis:

            arg 1  arg 2        arg 3      arg 4
   ./server <port> <executable> <argument> <argument> ...
 
   The first argument indicates a TCP/IP port as an integer 1-65535.
   
   The second argument indicates the executable to be run when a client
   connects.

   The subsequent arguments are passed to the executable program.

 */
int main(int argc, char **argv) {
  int sockfd, connfd;
  struct sockaddr_in serveraddr, clientaddr;
  socklen_t clientaddrlen;
  unsigned short int port;

  /* Check if enough arguments are given */
  if (argc < 3) {
    fprintf(stderr, "Not enough arguments.\n");
    return 1;
  }

  /* Check if first argument converts to an unsigned short */
  if (try_convert_unsigned_short(&port, argv[1]) < 0) {
    fprintf(stderr, "Cannot convert \"%s\" to a port number.\n", argv[1]);
    return -1;
  }
  
  /* Create a socket: IPv4, TCP */
  sockfd = /* TODO */;
  if (sockfd < 0) {
    fprintf(stderr, "Cannot create a socket: %s\n", strerror(errno));
    return 1;
  }

  /* Bind */
  memset(&serveraddr, 0, sizeof(serveraddr));
  /* TODO */
  if (/* TODO */) {
    fprintf(stderr, "Cannot bind: %s\n", strerror(errno));
    if (close(sockfd) < 0) {
      fprintf(stderr, "Cannot close a socket: %s\n", strerror(errno));
    }
    return 1;
  }

  /* Listen */
  if (/* TODO */) {
    fprintf(stderr, "Cannot listen: %s\n", strerror(errno));
    if (close(sockfd) < 0) {
      fprintf(stderr, "Cannot close a socket: %s\n", strerror(errno));
    }
    return 1;
  }

  /* Accept */
  clientaddrlen = sizeof(clientaddr);
  connfd = /* TODO */
  if (connfd < 0) {
    fprintf(stderr, "Cannot accept a connection: %s\n", strerror(errno));
    if (close(sockfd) < 0) {
      fprintf(stderr, "Cannot close a socket: %s\n", strerror(errno));
    }
    return 1;    
  }
  if (((size_t) clientaddrlen) != sizeof(clientaddr)) {
    fprintf(stderr, "Could not get full client address\n");
    if (close(connfd) < 0) {
      fprintf(stderr, "Cannot close a connection: %s\n", strerror(errno));
    }    
    if (close(sockfd) < 0) {
      fprintf(stderr, "Cannot close a socket: %s\n", strerror(errno));
    }
    return 1;    
  }

  /* Work with client */
  if (run_server(connfd, argc-2, &argv[2]) < 0) {
    if (close(connfd) < 0) {
      fprintf(stderr, "Cannot close a connection: %s\n", strerror(errno));
    }
    if (close(sockfd) < 0) {
      fprintf(stderr, "Cannot close a socket: %s\n", strerror(errno));
    }
    return 1;
  }
  
  /* Close connection */
  if (close(connfd) < 0) {
    fprintf(stderr, "Cannot close a connection: %s\n", strerror(errno));
    if (close(sockfd) < 0) {
      fprintf(stderr, "Cannot close a socket: %s\n", strerror(errno));
    }
    return 1;
  }
  
  /* Close the socket */
  if (close(sockfd) < 0) {
    fprintf(stderr, "Cannot close a socket: %s\n", strerror(errno));
    return 1;
  }
  
  /* Signal success */
  return 0;
}


