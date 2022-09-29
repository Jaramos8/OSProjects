// Kevin Clemons - Roberto Carrasco - Joel Ramos

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define BUFFER_LEN (4096)
#define FIRST_LINE_ALLOC_SIZE ((size_t) 2)
#define FIRST_ARRAY_SIZE ((size_t) 2)

/* 
Calls write() until all bytes are written or 
until an error occurs. Returns 0 on success -1 on failure
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


unsigned int myStrlen(const char *str)
{
    unsigned int len = 0;
    while(*str != '\0')
    {
        len++;
        str++;
    }
    return len;
}

int error(int err, char *msg){
  char *error = 0;
  int len;
  if(err >= 0){
    error = strerror(err);
    len = myStrlen(error);
    write(1, error, len);
  }
  len = myStrlen(msg);
  write(1, msg, len);
  return -1;
}


int myAtoi(char *str)
{
 int res = 0; // Initialize result

 // Iterate through all characters of input string and
 // update result
 for (int i = 0; str[i] != '\0'; ++i) {
     if (str[i]> '9' || str[i]<'0')
         return -1;
     res = res*10 + str[i] - '0';
 }

 // return result.
 return res;
}



int main(int argc, char **argv){
    char buffer[BUFFER_LEN];
    ssize_t read_res; /* signed size */
    size_t bytes_read; /* unsigned size */
    size_t i;
    char c;
    char *current_line;
    size_t current_line_len, current_line_size;
    char *ptr;
    char **lines;
    char **lptr;
    char *llptr;
    size_t lines_len, lines_size;
    char *lines_lengths;
    int tail_len;
    int fd;
    char *filename;
    

    fd = 0;
    tail_len = 10;
    current_line_len = (size_t) 0;
    current_line_size = (size_t) 0;
    current_line = NULL;    
    lines_len = (size_t) 0;
    lines_size = (size_t) 0;
    lines = NULL;
    lines_lengths = NULL;

  
    /***********Handle arguements *****************/
    
    if(argc  > 4){
      return error(-1,"Too many arguements\n");
    }else{
      for(int i = 1; i < argc; i++){
	if(argv[i][0] ==  '-' && argv[i][1] == 'n' && i+1 < argc){
	  tail_len = myAtoi( argv[i+1]);
	  i++;
	  
	}else{
	  filename = argv[i];
	  fd = open(filename, O_RDONLY);
	  if(fd < 0 ){
	    //throw error**********
	  }
	}
      }
    }

    
/*********************Read file until EOF*****************/
    while(1){


      
    /**** STEP 1:read into the buffer, up to sizeof(buffer) bytes ****/
        read_res = read(fd, buffer, sizeof(buffer));
	
        /* Handle return values of read system call */        
        /*If return value zero, done and end-of-file*/
        if (read_res == ((ssize_t) 0)) break;
        /* If return value negative, error and  die */
        if (read_res < ((ssize_t) 0)) {
	    error(errno, "Error Reading");
            /* Deallocate everything that has been allocated */
	    for (i=(size_t) 0; i<lines_len; i++){
                free(lines[i]);
            }
            free(lines);
            free(lines_lengths);
            return 1; 
        }
	
        /* read_res is positive */
        bytes_read = (size_t) read_res;


	
	
	
     /**** STEP 2: handle input and put into memory ****/
        for(i=(size_t) 0; i < bytes_read; i++){
            /* Get curr char */
            c = buffer[i];
            
            /* Put the current character into the current line */
            if ((current_line_len + ((size_t) 1)) > current_line_size){
                /* Allocate Memory */
                if ((current_line_len == ((size_t) 0)) ||
                    (current_line == NULL)) {
                    /* First allocation */ 
                    ptr = (char *) malloc(FIRST_LINE_ALLOC_SIZE * sizeof(char)); //Each char is 1 byte
                    if (ptr == NULL) {
			error(-1,"Could not allocate any more memory\n");
                        /* Deallocate everything that has been allocated */
                        for (i=(size_t) 0; i<lines_len; i++){
                            free(lines[i]);
                        }
                        free(lines);
                        free(lines_lengths);
                        return 1;
                    }
                    current_line = ptr;
                    current_line_size = FIRST_LINE_ALLOC_SIZE;
                } else{
                    /* Reallocation */
                    ptr = (char *) realloc(current_line, current_line_size * ((size_t) 2) * sizeof(char));
                     /* TODO: check overflow of the multiplication */
                    if (ptr == NULL){
			error(-1,"Could not allocate any more memory\n");
                        /* Deallocate everything that has been allocated */
                        for (i=(size_t) 0; i<lines_len; i++){
                            free(lines[i]);
                        }
                        free(lines);
                        free(lines_lengths);
                        return 1;
                    }
                    current_line = ptr;
                    current_line_size *= (size_t) 2;
                }
            }
            
            /* Here, we are sure to have the right space in memory. 
            We put the character in and increment the length */
            current_line[current_line_len] = c;
            current_line_len++;
            
            
            /* If this is a newline character, start a new line */
            if (c == '\n'){
                if ((lines_len + ((size_t) 1)) > lines_size) {
                    /* Allocate memory :) */
                    if ((lines_len == ((size_t) 0)) ||
                    (lines == NULL)) {
                        /* First allocation */
                        lptr = (char **) malloc(FIRST_ARRAY_SIZE * sizeof(char *));
                        if(lptr == NULL){
			    error(-1,"Could not allocate any more memory\n");
                            /* Deallocate everything that has been allocated */
                            for (i=(size_t) 0; i<lines_len; i++){
                                free(lines[i]);
                            }
                            free(lines);
                            free(lines_lengths);
                            return 1;
                        }
                        lines = lptr;
                        lines_size = FIRST_ARRAY_SIZE;
                        llptr = (char *) malloc(FIRST_ARRAY_SIZE * sizeof(char));
                        if(llptr == NULL){
                            error(-1,"Could not allocate any more memory\n");
                            /* Deallocate everything that has been allocated */
                            for (i=(size_t) 0; i<lines_len; i++){
                                free(lines[i]);
                            }
                            free(lines);
                            free(lines_lengths);
                            return 1;
                        }
                        lines_lengths = llptr;
                    } else {
                        /* Reallocation */
                        lptr = (char **) realloc(lines, lines_size * ((size_t) 2) * sizeof(char *));
                        
                        if(lptr == NULL){
			    error(-1,"Could not allocate any more memory\n");
                            /* Deallocate everything that has been allocated */
                            for (i=(size_t) 0; i<lines_len; i++){
                                free(lines[i]);
                            }
                            free(lines);
                            free(lines_lengths);
                            return 1;
                        }
                        lines = lptr;
                        lines_size *= (size_t) 2;
                        
                        /* Reallocation */
                        llptr = (char *) realloc(lines_lengths, lines_size * ((size_t) 2) * sizeof(char));
                        
                        if(llptr == NULL){
			    error(-1,"Could not allocate any more memory\n");
                            /* Deallocate everything that has been allocated */
                            for (i=(size_t) 0; i<lines_len; i++){
                                free(lines[i]);
                            }
                            free(lines);
                            free(lines_lengths);
                            return 1;
                        }
                        lines_lengths = llptr;
                    }
                }
                lines[lines_len] = current_line;
                lines_lengths[lines_len] = current_line_len;
                lines_len++;
                current_line = NULL;
                current_line_len = (size_t) 0;
                current_line_size = (size_t) 0;
            }
        }
        
    }
    
    /* In the case when the last line has no new line character at the
    end we need to put the line into the array of lines nevertheless */
    if ( !((current_line == NULL) || (current_line_len == (size_t) 0)) ) {
        if ((lines_len + ((size_t) 1)) > lines_size) {
            /* Allocate memory :) */
            if ((lines_len == ((size_t) 0)) ||
            (lines == NULL)) {
                /* First allocation */
                lptr = (char **) malloc(FIRST_ARRAY_SIZE * sizeof(char *));
                if(lptr == NULL){
		    error(-1,"Could not allocate any more memory\n");
                    /* Deallocate everything that has been allocated */
                    for (i=(size_t) 0; i<lines_len; i++){
                        free(lines[i]);
                    }
                    free(lines);
                    free(lines_lengths);
                    return 1;
                }
                lines = lptr;
                lines_size = FIRST_ARRAY_SIZE;
                llptr = (char *) malloc(FIRST_ARRAY_SIZE * sizeof(char));
                if(llptr == NULL){
       		    error(-1,"Could not allocate any more memory\n");
                   
                    /* Deallocate everything that has been allocated */
                    for (i=(size_t) 0; i<lines_len; i++){
                        free(lines[i]);
                    }
                    free(lines);
                    free(lines_lengths);
                    return 1;
                }
                lines_lengths = llptr;
            } else {
                /* Reallocation */
                lptr = (char **) realloc(lines, lines_size * ((size_t) 2) * sizeof(char *));

                if(lptr == NULL){
		    error(-1,"Could not allocate any more memory\n");
                    
                    /* Deallocate everything that has been allocated */
                    for (i=(size_t) 0; i<lines_len; i++){
                        free(lines[i]);
                    }
                    free(lines);
                    free(lines_lengths);
                    return 1;
                }
                lines = lptr;
                lines_size *= (size_t) 2;

                /* Reallocation */
                llptr = (char *) realloc(lines_lengths, lines_size * ((size_t) 2) * sizeof(char));
                        
                if(llptr == NULL){
		    error(-1,"Could not allocate any more memory\n");
                    
                    /* Deallocate everything that has been allocated */
                    for (i=(size_t) 0; i<lines_len; i++){
                        free(lines[i]);
                    }
                    free(lines);
                    free(lines_lengths);
                    return 1;
                }
                lines_lengths = llptr;
            }
                    
            lines[lines_len] = current_line;
            lines_lengths[lines_len] = current_line_len;
            lines_len++;
            current_line = NULL;
            current_line_len = (size_t) 0;
            current_line_size = (size_t) 0;
        }
    }

    /* Here, we have an array lines of "strings", i.e. array lines of 
    pointers to characters. There are lines_len valid entries */
    size_t offset;
    if(tail_len >= lines_len){
      offset = (size_t)0;
    }else{
      offset = lines_len - tail_len;
    }
    for (i = offset; i<lines_len; i++){
        my_write(1, lines[i], lines_lengths[i]);
    }
    
    /* Deallocate everything that has been allocated */
    for (i=(size_t) 0; i<lines_len; i++){
        free(lines[i]);
    }
    free(lines);
    free(lines_lengths);
    
    /* Signal success */
  return 0;
}
