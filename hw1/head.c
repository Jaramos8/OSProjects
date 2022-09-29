#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define LEN 4096

/**********************************
 *Input: a string
 *
 *return: an int representing the number
 *of characters in the string
 **********************************/
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

/**********************************
 *Input: errno or -1 if strerror is not
 *needed
 *
 *return: -1, prints an error message
 **********************************/
int error(int err){
  char *error = 0;
  int len;
  char *usage = "\nUsage: head <file>\n   or: head <file> -n <number of characters>\n";
  if(err >= 0){
    error = strerror(err);
    len = myStrlen(error);
    write(1, error, len);
  }
  len = myStrlen(usage);
  write(1, usage, len);
  return -1;
}

/**********************************
 *Input: a string representation of an
 *integer
 *
 *return: an integer
 **********************************/
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

int main(int argc, char* argv[]){
  char* path;
  char buf[LEN];
  int strtoint; //converting input into an integer
  int numlines = 10; //default if no argument is used
  int fd = 0;
  int writeRes;
  
  /*parses input*/
  if(argc == 2){
    path = argv[1];
    fd = open(path, O_RDONLY);
    if (fd == -1) {
      return error(errno);
    }
  }else if(argc > 4 || argc == 3){
    return error(-1);
  }else{
    for(int i = 1; i < argc; i++){
      switch(argv[i][0]){
      case '-':
	if((argv[i][1]) == 'n'){
	  strtoint = myAtoi(argv[i+1]);
	  numlines = strtoint;
	  if(numlines <= 0){
	    return 0;
	  }
	  if(myStrlen(argv[i]) > 2){
	    return error(-1);
	  }
	  i++;
	  break;
	}
	else{
	  return error(-1);
	}
      default:
	path = argv[i];
	fd = open(path, O_RDONLY);
	if (fd == -1) {
	  return error(errno);
	}
	break;
      }
    }
  }
  
  int bytes_read;
  int newline_count = 0; //to keep track of newline chars
  int written = 0; // determines whether the file was written or not
  int buffsize = 0; //stores the size of the buffer in case the input is larger than a file

  
  //reads file into buffer and prints to stdout  once the specified number of 
  //lines is reached
  while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) {
    if(bytes_read == -1){
      return error(errno);
    }
    for (int i =0; i < bytes_read; ++i) {
      //if at a newline or EOF
      if (buf[i] == '\n' || buf[i] == '\0') {
	newline_count += 1;
	buffsize = i;

	//if the specified number of lines is met..
	//print and break the loop
        if(newline_count >= numlines) {
	  writeRes = write(1,buf,i+1);
	  if(writeRes < 0){
	    return error(errno);
	  }
	  written = 1;
          break;
	}
      }
    }
  }
  //if number of lines from input is greater than the file, outputs the entire
  //file
  if(newline_count <= numlines && written == 0) {
    write(1,buf,buffsize+1);
    if(writeRes < 0){
      return error(errno);
    }
    written = 1;
  }
  close(fd);
  return 0;
}
