#ifndef FSTREE
#define FSTREE

#include "myfsdisk.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

// used to represent a file
struct myfsfile{
    long int offset;    // Offset within the file
	off_t size;         // Size of the file
	char * path;        // Path upto file
	char * name;        // Name of the file
	char * data;        // Data 
};


// used to represent a directory
struct myfstree{
    char * name;                    // Name of the file / directory
    char * path;                    // Path upto node
    char * type;                    // Type : "directory" or "file"
    int num_children;               // Number of children nodes
    int num_files;		            // Number of files
    struct myfstree * parent;         // Pointer to parent node
    struct myfstree ** children;      // Pointers to children nodes
    struct myfsfile ** fchildren;     // Pointers to files in the directory
    uid_t user_id;		            // userid
    gid_t group_id;		            // groupid
    mode_t permissions;		        // Permissions
    time_t c_time;                  // Status change time
    time_t a_time;                  // Access time
    time_t m_time;                  // Modified time
    time_t b_time;                  // Creation time
    off_t size;                     // Size of the node
    unsigned long int inode_number; // Inode number of the node in disk    
};

typedef struct myfstree myfstree;
typedef struct myfsfile myfsfile;


extern time_t t;

extern myfstree * root;


//get previous directory
char * extract_path(char ** copy_path);

//reverse a string
char * reverse(char * str, int mode);


// get current directory
char * extract_dir(char ** copy_path);


//search for a node in myfstree
myfstree * search_node(char * path);


// initialise tree node
myfstree * init_node(const char * path, char * name, myfstree * parent,int type);


// insert a node into myfstree
void insert_node(const char * path);


// intialise file node
myfsfile * init_file(const char * path,char * name);

//delete file
void delete_file(const char *path);


//delete a node.
int delete_node(const char * path);

// insert file into myfstree
void insert_file(const char * path);


//search for a file in myfstree that Returns a pointer to the file
myfsfile * find_file(const char * path);


// Moves a file or directory from src to dst
void move_node(const char * from,const char * to);


// used by move to update paths
void path_update(myfstree * dir_node,char * topath);


// Loads a tree node into the tree structure
void load_node(char * path, char * type, gid_t groupid, uid_t userid, time_t lc_time, time_t lm_time, time_t la_time, time_t lb_time, unsigned long int inode, off_t size, mode_t lpermissions);


// Loads file for read/write
void load_file(const char *path, char * data);

#endif
