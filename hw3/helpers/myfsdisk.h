#ifndef DISKOPS
#define DISKOPS

#include "myfstree.h"
#include "myfsbitmap.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <math.h>

typedef struct myfstree myfstree;
typedef struct myfsfile myfsfile;

//Initial size of the data and metadata disk files when created
#define INIT_SIZE 1048576

//Block size of the metadata disk file
#define BLOCK_SIZE 4096

//Open marker - Defines the beginning of a node in the metadata disk file
#define OPEN_MARKER "{\n"

//Close marker - Defines the end of a node in the metadata disk file
#define CLOSE_MARKER "}\n"

//Data bitmap is stored here when loaded into memory
extern uint8_t * datamap;

//Size of the metadata disk bitmap stored here when loaded into memory
extern uint64_t metamap_size;

//Data fd
extern int data_fd;

//metadata fd
extern int meta_fd;

//data disk bitmap size stored here when loaded
extern uint64_t datamap_size;

//Metadata disk bitmap is stored here when loaded into memory
extern uint8_t * metamap;


// Reset data fd & mark file closed
void resetdatafd();


// Resets the metadata fd and marks it closed
void resetmetafd();


// Writes the bitmap to a file corresponding to the file descriptor 'fd'
void writebitmap(int fd, uint8_t * bitmap, uint64_t bitmap_size);


// Loads the bitmap corresponding to the file given by 'fd' into memory
void loadbitmap(int fd, uint8_t ** bitmap, uint64_t * bitmap_size);


// Creates the metadata & data disk files or loads the metadata into memory
int createdisk();


// Finds first available block
unsigned long int find_free_block(uint8_t * bitmap, uint64_t bitmap_size);


// Writes a tree node to the diskfile using fd
void write_diskfile(int fd, uint8_t * bitmap, uint64_t bitmap_size, myfstree * node);


// processes info pertaining to the tree node
void serialize_metadata(myfstree * temp);


// Wrapper
void serialize_metadata_wrapper(myfstree * node);


// Fetches the block number of the parent node
unsigned long int get_parent_block(int fd, myfstree * node, int child_blocknumber);

unsigned long int get_chained_meta_block(int fd, unsigned long int parent_blocknumber, unsigned long int child_blocknumber);


// Updates a given tree node
int update_node(int fd, uint8_t * bitmap, uint64_t bitmap_size, myfstree * node, int mode);


// Wrapper function for update_node
int update_node_wrapper(myfstree * node, int mode);


// Load the metadata from disk into memory
void deserialize_metadata(unsigned long int blknumber);


// Wrapper for deserialize_metadata
void deserialize_metadata_wrapper();


// Deletes a block given by the inode number
void delete_metadata_block(char * type,unsigned long int blocknumber);


// Checks if a block is valid
int check_validity_block(unsigned long int blocknumber);


// Writes to the data disk file
void write_data(int fd, uint8_t * bitmap, uint64_t bitmap_size,unsigned long int inode,char * data,myfstree * node);


// Serialises file data
void serialize_filedata(unsigned long int inode,char * data,myfstree * node);


// Wrapper
void serialize_filedata_wrapper(unsigned long int inode,char * data,myfstree * node);


// Fetch the block number of a given data block corresponding to a particular file
unsigned long int find_data_block(unsigned long int blocknumber);


// Loads the file data into memory upon read / write requests
char * deserialize_file_data(unsigned long int inode);

#endif
