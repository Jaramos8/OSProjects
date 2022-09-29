#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

typedef struct{
	char phone[6];
	char location[25];
	char newline;
} entry_t;


void usage(){
	fprintf(stderr, "lookup <filename> <number>\n");
}

unsigned int myStrlen(const char *str){
	unsigned int len = 0;
	       
       	while(*str != '\0'){			  
	 len++;				           
	 str++;						       
       	}	   	 
	return len;
}

int error(int err){
	char *error = 0;
	int len;
	if(err >= 0){
		error = strerror(err);
		len = myStrlen(error);
		write(1, error, len);
	}

	printf(" \n");
	return -1;
}

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

       	bytes_to_be_written -= (size_t) bytes_written_this_time;								bytes_already_written += (size_t) bytes_written_this_time;
									        
       	}			   
       	return 0;
}


int compare_entries(const char *number_a, const char *number_b){
	const char *a;
	const char *b;

	for(a = number_a, b = number_b;
			((*a != '\0') && (*a != '\n') && 
			 (*b != '\0') && (*b != '\n')); 
			a++, b++){
		if (*a < *b) return -1;
		if (*a > *b) return 1;
	}
	if (((*a == '\0') || (*a == '\n')) && 
		((*b != '\0') && (*b != '\n'))){
		return -1;
	}
	
	if (((*a != '\0') || (*a != '\n')) && 
		((*b == '\0') && (*b == '\n'))){
		return 1;
	}
	return 0;
}
char *lookup_doit(entry_t dictionary[], ssize_t num_entries, const char *number){
	ssize_t l,r,m;
	int cmp;

	l= (ssize_t) 0;
	r= num_entries - ((ssize_t) 1);
	while(l <= r){
		m = (l+r)/((ssize_t) 2);
		cmp = compare_entries(dictionary[m].phone, number);
		if(cmp == 0) return dictionary[m].location;
		if(cmp < 0){
			l = m + ((ssize_t) 1);	
		}else{
		r = m - ((ssize_t) 1);
		}
	}
	return NULL;
}

int lookup(const char *filename, const char *number){
	int fd;
	off_t lseek_res;
	size_t filesize;
	void *ptr;
	char *location;
	
	if(myStrlen(number)<6){
		return error(22);
	}


	fd = open(filename, O_RDONLY);
	if(fd<0){
		return error(2);
	}

	lseek_res = lseek(fd,(off_t) 0, SEEK_END);
	if(lseek_res == ((off_t) -1)){
		return error(29);

		if(close(fd)<0){	
			return error(22);

		}

		return 1;
	}

	filesize = (size_t) lseek_res;
	if((filesize % 32) != ((size_t)0)){
		return error(29);


		if(close(fd)<0){	
			return error(22);

		}
		return 1;
	}

	ptr = mmap(NULL, filesize, PROT_READ, MAP_SHARED, fd, (off_t) 0);
	if(ptr == MAP_FAILED){
		return error(22);


		if(close(fd)<0){	
		return error(22);

		}
		return 1;
	}

	location = lookup_doit((entry_t *) ptr, ((ssize_t) filesize / sizeof(entry_t)), number);
	if(location == NULL){

	return error(1);

	
	}
	else{
		my_write(1,number,6);
		my_write(1," = ",5 );
		my_write(1,location,25);
		printf("\n");
	}

	if(munmap(ptr,filesize) < 0){
		return error(-1);

		if(close(fd)<0){	
			return error(-1);

		}
		return 1;
	}

	if(close(fd)<0){	
	return error(22);

	}
	return 0;
}

int main(int argc, char **argv){
	char *filename;
	char *number;
	
	if (argc<3){
	usage();
	return 1;
	}
	
	filename = argv[1];
	number = argv[2];

	if(lookup(filename, number)) return 1;

	return 0;
}


