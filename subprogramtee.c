/* Joel Ramos 10/03/2022 
 * My main source and basically only was the notes we have taken in class. basically, lookup, taco, pipes, children and dup. The only sources used online where sources who talked about certain system calls like, execvp and pipe.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

/* This my_write is used in part seven.
 */
int my_write(int fd, const char *buf, size_t bytes){
	size_t bytes_to_be_written;
	size_t bytes_already_written;
	ssize_t bytes_written_this_time;
	    
	bytes_to_be_written = bytes;
	bytes_already_written = (size_t) 0;
	while (bytes_to_be_written > ((size_t) 0)) {
		bytes_written_this_time = write(fd, &buf[bytes_already_written], bytes_to_be_written);
		if (bytes_written_this_time < ((size_t) 0)){
			return -1;
		}
		bytes_to_be_written -= (size_t) bytes_written_this_time;
		bytes_already_written += (size_t) bytes_written_this_time;
	}
	    
	return 0;
}


int main(int argc, char **argv){
	
	char *file;
	int fd;
	int pipefd[2];
	int reading, writing;
	pid_t pid;
	char buf[8192];
	size_t i;
	size_t read_res;	

/*This is part one where the program checks if there are at least 3 command line arguments, if not it gives an error.
 */
	if (argc<3){
		fprintf(stderr, "Not enough command line arguments. \n");		
		exit(1);
		return 1;
	}
/*This is part two where opens the file which is in argument argv[1], if it fails it gives an error. 
 */

	file = argv[1];
	fd = open(file, O_WRONLY);
	
	if(fd<0){
		fprintf(stderr, "Error opening file \"%s\": %s\n",file, strerror(errno));
		exit(1);
		return 1;
	}
/*This is part three where a pipe is created, if it fails it closes the file, and give out an error, I added erors to the close as the proffessor had errors in all the close he used in class. 
 */	
	if(pipe(pipefd) < 0 ){

		if(close(fd)<0){	
			fprintf(stderr, "Error closing file \"%s\": %s\n", file, strerror(errno));
		}

		fprintf(stderr, "pipe() failed : %s\n", strerror(errno));
		
		exit(1);
		return 1;
	}

	reading = pipefd[0];
	writing = pipefd[1];
	

/* This is part four where fork() is used to create a child process, if it fails both ends of the pipe are closed and   the file, and we give out an error. I added errors in the close of read and write.
*/
	
	pid = fork();
	if(pid < ((pid_t) 0)){	
		fprintf(stderr, "Fork Failed: %s\n", strerror(errno));

		if(close(writing)<0){
			fprintf(stderr, "Close Failed:  %s\n", strerror(errno));
		}

		if(close(reading)<0){
			fprintf(stderr, "Close Failed: %s\n", strerror(errno));
		}
		
		exit(1);
		return 1;
	}
/* This is part five where the child closes the read part of the pipe[0] and the file. it closes standard output and dups the write end of the pipe to be the new standard output. 
 */
	
	if(pid == ((pid_t) 0)){
	
		if(close(reading)<0){
			fprintf(stderr, "Close Failed %s\n", strerror(errno));
			exit(2);	
		}
	
		if(close(fd)<0){	
			fprintf(stderr, "Error closing file \"%s\": %s\n", file, strerror(errno));
			exit(2);
		}
	
		if(dup2(writing, 1) < 0){
			fprintf(stderr, "dup2 failed: %s\n", strerror(errno));
			exit(2);
		}
		
		if(close(writing)<0){
			fprintf(stderr, "Close Failed: %s\n", strerror(errno));
			exit(2);
		}
		

/* This is part six. where the child process  replaces itself with a new executabl given by the second argument argv[2], the executable arguments are given by the 3rd and 4th arguments if applicable
 */	

		if(execvp(argv[2], &argv[2]) < 0){
			fprintf(stderr, "execvp Failed: %s\n", strerror(errno));
			return 1;
		}
		
		return 1;
	} else {
	
/*This is part seven, where the parent closes the write end of the pipe, if it fails in prints out a message, if it     doesn't it continues and uses read() to read from the the read end of the pipe. it stops when End-of-file is reached. it writes everything it reads into both file and standard output with write() in function my_write().
 */
		if(close(writing)<0){
			fprintf(stderr, "Close Failed: %s\n", strerror(errno));
			return 1;
		}
	
		for( i = ((size_t) 0); i<sizeof(buf); i++){
			buf[i] = '\0';
		}
		
		
		read_res = read(reading,buf,sizeof(buf)-((size_t)1));

		if(read_res < ((ssize_t) 0)){
			fprintf(stderr, "Read Failed: %s\n", strerror(errno));
			
			if(close(reading)<0){
				fprintf(stderr, "Close Failed: %s\n", strerror(errno));
			}

			return 1;
		}
		
		my_write(1, buf, sizeof(buf));
		my_write(fd,buf, sizeof(buf));	

/* This is part eight,After reaching EOF parent waits for child to end and ir closes the file and exits.
 */

		waitpid(pid, NULL, 0);
		if(close(fd)<0){	
			fprintf(stderr, "Error closing file \"%s\": %s\n", file, strerror(errno));
		}
		exit(0);

		return 1;
	}

	return 0;
}
