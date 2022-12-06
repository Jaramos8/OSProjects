/*

  MyFS: a tiny file-system written for educational purposes

  MyFS is 

  Copyright 2018-21 by

  University of Alaska Anchorage, College of Engineering.

  Copyright 2022

  University of Texas at El Paso, Department of Computer Science.

  Contributors: Christoph Lauter
                ... 
                ... and
                ...

  and based on 

  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall myfs.c implementation.c `pkg-config fuse --cflags --libs` -o myfs

*/

#include <stddef.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include <fuse.h>


/* The filesystem you implement must support all the 13 operations
   stubbed out below. There need not be support for access rights,
   links, symbolic links. There needs to be support for access and
   modification times and information for statfs.

   The filesystem must run in memory, using the memory of size 
   fssize pointed to by fsptr. The memory comes from mmap and 
   is backed with a file if a backup-file is indicated. When
   the filesystem is unmounted, the memory is written back to 
   that backup-file. When the filesystem is mounted again from
   the backup-file, the same memory appears at the newly mapped
   in virtual address. The filesystem datastructures hence must not
   store any pointer directly to the memory pointed to by fsptr; it
   must rather store offsets from the beginning of the memory region.

   When a filesystem is mounted for the first time, the whole memory
   region of size fssize pointed to by fsptr reads as zero-bytes. When
   a backup-file is used and the filesystem is mounted again, certain
   parts of the memory, which have previously been written, may read
   as non-zero bytes. The size of the memory region is at least 2048
   bytes.

   CAUTION:

   * You MUST NOT use any global variables in your program for reasons
   due to the way FUSE is designed.

   You can find ways to store a structure containing all "global" data
   at the start of the memory region representing the filesystem.

   * You MUST NOT store (the value of) pointers into the memory region
   that represents the filesystem. Pointers are virtual memory
   addresses and these addresses are ephemeral. Everything will seem
   okay UNTIL you remount the filesystem again.

   You may store offsets/indices (of type size_t) into the
   filesystem. These offsets/indices are like pointers: instead of
   storing the pointer, you store how far it is away from the start of
   the memory region. You may want to define a type for your offsets
   and to write two functions that can convert from pointers to
   offsets and vice versa.

   * You may use any function out of libc for your filesystem,
   including (but not limited to) malloc, calloc, free, strdup,
   strlen, strncpy, strchr, strrchr, memset, memcpy. However, your
   filesystem MUST NOT depend on memory outside of the filesystem
   memory region. Only this part of the virtual memory address space
   gets saved into the backup-file. As a matter of course, your FUSE
   process, which implements the filesystem, MUST NOT leak memory: be
   careful in particular not to leak tiny amounts of memory that
   accumulate over time. In a working setup, a FUSE process is
   supposed to run for a long time!

   It is possible to check for memory leaks by running the FUSE
   process inside valgrind:

   valgrind --leak-check=full ./myfs --backupfile=test.myfs ~/fuse-mnt/ -f

   However, the analysis of the leak indications displayed by valgrind
   is difficult as libfuse contains some small memory leaks (which do
   not accumulate over time). We cannot (easily) fix these memory
   leaks inside libfuse.

   * Avoid putting debug messages into the code. You may use fprintf
   for debugging purposes but they should all go away in the final
   version of the code. Using gdb is more professional, though.

   * You MUST NOT fail with exit(1) in case of an error. All the
   functions you have to implement have ways to indicated failure
   cases. Use these, mapping your internal errors intelligently onto
   the POSIX error conditions.

   * And of course: your code MUST NOT SEGFAULT!

   It is reasonable to proceed in the following order:

   (1)   Design and implement a mechanism that initializes a filesystem
         whenever the memory space is fresh. That mechanism can be
         implemented in the form of a filesystem handle into which the
         filesystem raw memory pointer and sizes are translated.
         Check that the filesystem does not get reinitialized at mount
         time if you initialized it once and unmounted it but that all
         pieces of information (in the handle) get read back correctly
         from the backup-file. 

   (2)   Design and implement functions to find and allocate free memory
         regions inside the filesystem memory space. There need to be 
         functions to free these regions again, too. Any "global" variable
         goes into the handle structure the mechanism designed at step (1) 
         provides.

   (3)   Carefully design a data structure able to represent all the
         pieces of information that are needed for files and
         (sub-)directories.  You need to store the location of the
         root directory in a "global" variable that, again, goes into the 
         handle designed at step (1).
          
   (4)   Write __myfs_getattr_implem and debug it thoroughly, as best as
         you can with a filesystem that is reduced to one
         function. Writing this function will make you write helper
         functions to traverse paths, following the appropriate
         subdirectories inside the file system. Strive for modularity for
         these filesystem traversal functions.

   (5)   Design and implement __myfs_readdir_implem. You cannot test it
         besides by listing your root directory with ls -la and looking
         at the date of last access/modification of the directory (.). 
         Be sure to understand the signature of that function and use
         caution not to provoke segfaults nor to leak memory.

   (6)   Design and implement __myfs_mknod_implem. You can now touch files 
         with 

         touch foo

         and check that they start to exist (with the appropriate
         access/modification times) with ls -la.

   (7)   Design and implement __myfs_mkdir_implem. Test as above.

   (8)   Design and implement __myfs_truncate_implem. You can now 
         create files filled with zeros:

         truncate -s 1024 foo

   (9)   Design and implement __myfs_statfs_implem. Test by running
         df before and after the truncation of a file to various lengths. 
         The free "disk" space must change accordingly.

   (10)  Design, implement and test __myfs_utimens_implem. You can now 
         touch files at different dates (in the past, in the future).

   (11)  Design and implement __myfs_open_implem. The function can 
         only be tested once __myfs_read_implem and __myfs_write_implem are
         implemented.

   (12)  Design, implement and test __myfs_read_implem and
         __myfs_write_implem. You can now write to files and read the data 
         back:

         echo "Hello world" > foo
         echo "Hallo ihr da" >> foo
         cat foo

         Be sure to test the case when you unmount and remount the
         filesystem: the files must still be there, contain the same
         information and have the same access and/or modification
         times.

   (13)  Design, implement and test __myfs_unlink_implem. You can now
         remove files.

   (14)  Design, implement and test __myfs_unlink_implem. You can now
         remove directories.

   (15)  Design, implement and test __myfs_rename_implem. This function
         is extremely complicated to implement. Be sure to cover all 
         cases that are documented in man 2 rename. The case when the 
         new path exists already is really hard to implement. Be sure to 
         never leave the filessystem in a bad state! Test thoroughly 
         using mv on (filled and empty) directories and files onto 
         inexistant and already existing directories and files.

   (16)  Design, implement and test any function that your instructor
         might have left out from this list. There are 13 functions 
         __myfs_XXX_implem you have to write.

   (17)  Go over all functions again, testing them one-by-one, trying
         to exercise all special conditions (error conditions): set
         breakpoints in gdb and use a sequence of bash commands inside
         your mounted filesystem to trigger these special cases. Be
         sure to cover all funny cases that arise when the filesystem
         is full but files are supposed to get written to or truncated
         to longer length. There must not be any segfault; the user
         space program using your filesystem just has to report an
         error. Also be sure to unmount and remount your filesystem,
         in order to be sure that it contents do not change by
         unmounting and remounting. Try to mount two of your
         filesystems at different places and copy and move (rename!)
         (heavy) files (your favorite movie or song, an image of a cat
         etc.) from one mount-point to the other. None of the two FUSE
         processes must provoke errors. Find ways to test the case
         when files have holes as the process that wrote them seeked
         beyond the end of the file several times. Your filesystem must
         support these operations at least by making the holes explicit 
         zeros (use dd to test this aspect).

   (18)  Run some heavy testing: copy your favorite movie into your
         filesystem and try to watch it out of the filesystem.

*/

/* Helper types and functions */

#define MAX_FILES 1024
#define BLOCK_SIZE 4096

struct inode {
    int used;
    int mode;
    int size;
    int data_block;
};

/* Directory entry */
struct dirent {
    int inode;
    char name[256];
};

struct superblock {
    int inode_table_block;
    int data_block_start;
    int num_blocks;
    int num_inodes;
};

/* In-memory inode table */
static struct inode inode_table[MAX_FILES];

/* In-memory superblock */
static struct superblock superblock;

/* In-memory data blocks */
static char data_blocks[MAX_FILES][4096];

/* In-memory directory entries */
static struct dirent root_dir[MAX_FILES];

/* In-memory file descriptor table */
static int fd_table[MAX_FILES];

struct myfs_data {
    uint64_t fsid;
    uint64_t flags;
    int num_inodes;
    int num_data_blocks;
    int num_files;
    struct inode_table_entry *inode_table;
    struct data_block_table_entry *data_block_table;
    struct root_dir_entry *root_dir;
};

struct inode_table_entry {
    uint64_t inode_num;
    uint64_t size;
    uint64_t access_time;
    uint64_t modification_time;
    uint64_t *data_block;
};

struct data_block_table_entry {
    uint64_t block_num;
    uint8_t used;
    char data[BLOCK_SIZE];
};

struct root_dir_entry {
    char name[NAME_MAX];
    uint64_t inode;
};


/* End of helper functions */

/* Implements an emulation of the stat system call on the filesystem 
   of size fssize pointed to by fsptr. 
   
   If path can be followed and describes a file or directory 
   that exists and is accessable, the access information is 
   put into stbuf. 

   On success, 0 is returned. On failure, -1 is returned and 
   the appropriate error code is put into *errnoptr.

   man 2 stat documents all possible error codes and gives more detail
   on what fields of stbuf need to be filled in. Essentially, only the
   following fields need to be supported:

   st_uid      the value passed in argument
   st_gid      the value passed in argument
   st_mode     (as fixed values S_IFDIR | 0755 for directories,
                                S_IFREG | 0755 for files)
   st_nlink    (as many as there are subdirectories (not files) for directories
                (including . and ..),
                1 for files)
   st_size     (supported only for files, where it is the real file size)
   st_atim
   st_mtim

*/
int __myfs_getattr_implem(void *fsptr, size_t fssize, int *errnoptr,
                          uid_t uid, gid_t gid,
                          const char *path, struct stat *stbuf) {
       
     /* Cast the file system data structure pointer to the correct type */
    struct myfs_data *fsdata = (struct myfs_data *)fsptr;

    /* Check for the root directory */
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    /* Check for files in the root directory */
    for (int i = 0; i < fsdata->num_files; i++) {
        if (fsdata->root_dir[i].inode == 0)
            continue;

        if (strcmp(path, fsdata->root_dir[i].name) == 0) {
            stbuf->st_mode = S_IFREG | 0444;
            stbuf->st_nlink = 1;
            stbuf->st_size = fsdata->inode_table[fsdata->root_dir[i].inode].size;
            return 0;
        }
    }

    /* Return an error if the file or directory was not found */
    *errnoptr = ENOENT;
    return -1;
}

/* Implements an emulation of the readdir system call on the filesystem 
   of size fssize pointed to by fsptr. 

   If path can be followed and describes a directory that exists and
   is accessable, the names of the subdirectories and files 
   contained in that directory are output into *namesptr. The . and ..
   directories must not be included in that listing.

   If it needs to output file and subdirectory names, the function
   starts by allocating (with calloc) an array of pointers to
   characters of the right size (n entries for n names). Sets
   *namesptr to that pointer. It then goes over all entries
   in that array and allocates, for each of them an array of
   characters of the right size (to hold the i-th name, together 
   with the appropriate '\0' terminator). It puts the pointer
   into that i-th array entry and fills the allocated array
   of characters with the appropriate name. The calling function
   will call free on each of the entries of *namesptr and 
   on *namesptr.

   The function returns the number of names that have been 
   put into namesptr. 

   If no name needs to be reported because the directory does
   not contain any file or subdirectory besides . and .., 0 is 
   returned and no allocation takes place.

   On failure, -1 is returned and the *errnoptr is set to 
   the appropriate error code. 

   The error codes are documented in man 2 readdir.

   In the case memory allocation with malloc/calloc fails, failure is
   indicated by returning -1 and setting *errnoptr to EINVAL.

*/
int __myfs_readdir_implem(void *fsptr, size_t fssize, int *errnoptr,
                          const char *path, char ***namesptr) {

    /* Cast the file system data structure pointer to the correct type */
    struct myfs_data *fsdata = (struct myfs_data *)fsptr;

    /* Allocate memory for the array of file and directory names */
    *namesptr = malloc(sizeof(char *) * fsdata->num_files);

    /* Check for the root directory */
    if (strcmp(path, "/") == 0) {
        /* Add the "." and ".." entries to the array of names */
        (*namesptr)[0] = strdup(".");
        (*namesptr)[1] = strdup("..");

        /* Add the entries in the root directory to the array of names */
        int index = 2;
        for (int i = 0; i < fsdata->num_files; i++) {
            if (fsdata->root_dir[i].inode == 0)
                continue;

            (*namesptr)[index] = strdup(fsdata->root_dir[i].name);
            index++;
        }

        return index;
    }

    /* Return an error if the directory was not found */
    *errnoptr = ENOENT;
    return -1;
}

/* Implements an emulation of the mknod system call for regular files
   on the filesystem of size fssize pointed to by fsptr.

   This function is called only for the creation of regular files.

   If a file gets created, it is of size zero and has default
   ownership and mode bits.

   The call creates the file indicated by path.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 mknod.

    */
int __myfs_mknod_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path) {
      /* Cast the file system data structure pointer to the correct type */
    struct myfs_data *fsdata = (struct myfs_data *)fsptr;

    /* Check if the file already exists */
    for (int i = 0; i < fsdata->num_files; i++) {
        if (strcmp(path, fsdata->root_dir[i].name) == 0) {
            *errnoptr = EEXIST;
            return -1;
        }
    }

    /* Check if there is space for a new file in the file system */
    if (fsdata->num_files >= MAX_FILES) {
        *errnoptr = ENOSPC;
        return -1;
    }

    /* Allocate space for the new file in the root directory */
    strncpy(fsdata->root_dir[fsdata->num_files].name, path, 255);
    fsdata->root_dir[fsdata->num_files].name[255] = '\0';

    /* Allocate space for the new file in the inode table */
    fsdata->inode_table[fsdata->num_files].used = 1;
    fsdata->inode_table[fsdata->num_files].mode = 0444;
    fsdata->inode_table[fsdata->num_files].size = 0;
    fsdata->inode_table[fsdata->num_files].data_block = 0;

    /* Update the root directory entry to point to the new inode */
    fsdata->root_dir[fsdata->num_files].inode = fsdata->num_files;

    /* Increment the number of files in the file system */
    fsdata->num_files++;

    return 0;
}

/* Implements an emulation of the unlink system call for regular files
   on the filesystem of size fssize pointed to by fsptr.

   This function is called only for the deletion of regular files.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 unlink.

*/
int __myfs_unlink_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path) {
    /* Cast the file system data structure pointer to the correct type */
    struct myfs_data *fsdata = (struct myfs_data *)fsptr;

    /* Look up the file in the root directory */
    for (int i = 0; i < fsdata->num_files; i++) {
        if (strcmp(path, fsdata->root_dir[i].name) == 0) {
            /* Free the file's inode and data block */
            fsdata->inode_table[fsdata->root_dir[i].inode].used = 0;
            memset(fsdata->data_block_table[fsdata->inode_table[fsdata->root_dir[i].inode].data_block], 0, BLOCK_SIZE);

            /* Clear the root directory entry for the file */
            memset(&fsdata->root_dir[i], 0, sizeof(struct root_dir_entry));

            return 0;
        }
    }

    /* Return an error if the file was not found */
    *errnoptr = ENOENT;
    return -1;
}

/* Implements an emulation of the rmdir system call on the filesystem 
   of size fssize pointed to by fsptr. 

   The call deletes the directory indicated by path.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The function call must fail when the directory indicated by path is
   not empty (if there are files or subdirectories other than . and ..).

   The error codes are documented in man 2 rmdir.

*/
int __myfs_rmdir_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path) {
     /* Cast the file system data structure pointer to the correct type */
    struct myfs_data *fsdata = (struct myfs_data *)fsptr;

    /* Check if the directory is empty */
    for (int i = 0; i < fsdata->num_files; i++) {
        if (strcmp(path, fsdata->root_dir[i].name) == 0) {
            /* Check if the directory has any entries */
            if (fsdata->root_dir[i].inode != 0) {
                *errnoptr = ENOTEMPTY;
                return -1;
            }

            /* Free the directory's inode and data block */
            fsdata->inode_table[fsdata->root_dir[i].inode].used = 0;
            memset(fsdata->data_block_table[fsdata->inode_table[fsdata->root_dir[i].inode].data_block], 0, BLOCK_SIZE);

            /* Clear the root directory entry for the directory */
            memset(&fsdata->root_dir[i], 0, sizeof(struct root_dir_entry));

            return 0;
        }
    }

    /* Return an error if the directory was not found */
    *errnoptr = ENOENT;
    return -1;
}

/* Implements an emulation of the mkdir system call on the filesystem 
   of size fssize pointed to by fsptr. 

   The call creates the directory indicated by path.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 mkdir.

*/
int __myfs_mkdir_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path) {
    /* Cast the file system data structure pointer to the correct type */
    struct myfs_data *fsdata = (struct myfs_data *)fsptr;

    /* Check if the directory already exists */
    for (int i = 0; i < fsdata->num_files; i++) {
        if (strcmp(path, fsdata->root_dir[i].name) == 0) {
            *errnoptr = EEXIST;
            return -1;
        }
    }

    /* Check if there is space for a new directory in the file system */
    if (fsdata->num_files >= MAX_FILES) {
        *errnoptr = ENOSPC;
        return -1;
    }

    /* Allocate space for the new directory in the root directory */
    strncpy(fsdata->root_dir[fsdata->num_files].name, path, 255);
    fsdata->root_dir[fsdata->num_files].name[255] = '\0';

    /* Allocate space for the new directory in the inode table */
    fsdata->inode_table[fsdata->num_files].used = 1;
    fsdata->inode_table[fsdata->num_files].mode = 0444;
    fsdata->inode_table[fsdata->num_files].size = 0;
    fsdata->inode_table[fsdata->num_files].data_block = 0;

    /* Update the root directory entry to point to the new inode */
    fsdata->root_dir[fsdata->num_files].inode = fsdata->num_files;

    /* Increment the number of files in the file system */
    fsdata->num_files++;

    return 0;
}

/* Implements an emulation of the rename system call on the filesystem 
   of size fssize pointed to by fsptr. 

   The call moves the file or directory indicated by from to to.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   Caution: the function does more than what is hinted to by its name.
   In cases the from and to paths differ, the file is moved out of 
   the from path and added to the to path.

   The error codes are documented in man 2 rename.

*/
int __myfs_rename_implem(void *fsptr, size_t fssize, int *errnoptr,
                         const char *from, const char *to) {
  /* Cast the file system data structure pointer to the correct type */
    struct myfs_data *fsdata = (struct myfs_data *)fsptr;

    /* Look up the file or directory in the root directory */
    for (int i = 0; i < fsdata->num_files; i++) {
        if (strcmp(from, fsdata->root_dir[i].name) == 0) {
            /* Check if the new name is already in use */
            for (int j = 0; j < fsdata->num_files; j++) {
                if (strcmp(to, fsdata->root_dir[j].name) == 0) {
                    *errnoptr = EEXIST;
                    return -1;
                }
            }

            /* Update the root directory entry with the new name */
            strncpy(fsdata->root_dir[i].name, to, 255);
            fsdata->root_dir[i].name[255] = '\0';

            return 0;
        }
    }

    /* Return an error if the file or directory was not found */
    *errnoptr = ENOENT;
    return -1;
}

/* Implements an emulation of the truncate system call on the filesystem 
   of size fssize pointed to by fsptr. 

   The call changes the size of the file indicated by path to offset
   bytes.

   When the file becomes smaller due to the call, the extending bytes are
   removed. When it becomes larger, zeros are appended.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 truncate.

*/
int __myfs_truncate_implem(void *fsptr, size_t fssize, int *errnoptr,
                           const char *path, off_t offset) {
     /* Cast the file system data structure pointer to the correct type */
    struct myfs_data *fsdata = (struct myfs_data *)fsptr;

    /* Look up the file in the root directory */
    for (int i = 0; i < fsdata->num_files; i++) {
        if (strcmp(path, fsdata->root_dir[i].name) == 0) {
            /* Update the file's size in the inode table */
            fsdata->inode_table[fsdata->root_dir[i].inode].size = offset;

            /* Check if the file's data blocks need to be updated */
            if (offset > fsdata->inode_table[fsdata->root_dir[i].inode].size) {
                /* Allocate additional data blocks if necessary */
                int num_blocks = offset / BLOCK_SIZE;
                if (offset % BLOCK_SIZE != 0) {
                    num_blocks++;
                }

                for (int j = fsdata->inode_table[fsdata->root_dir[i].inode].data_block; j < num_blocks; j++) {
                    fsdata->data_block_table[j].used = 1;
                }
            } else if (offset < fsdata->inode_table[fsdata->root_dir[i].inode].size) {
                /* Free any unused data blocks */
                int num_blocks = offset / BLOCK_SIZE;
                if (offset % BLOCK_SIZE != 0) {
                    num_blocks++;
                }

                for (int j = num_blocks; j < fsdata->inode_table[fsdata->root_dir[i].inode].data_block; j++) {
                    fsdata->data_block_table[j].used = 0;
                }
            }

            return 0;
        }
    }

    /* Return an error if the file was not found */
    *errnoptr = ENOENT;
    return -1;
}

/* Implements an emulation of the open system call on the filesystem 
   of size fssize pointed to by fsptr, without actually performing the opening
   of the file (no file descriptor is returned).

   The call just checks if the file (or directory) indicated by path
   can be accessed, i.e. if the path can be followed to an existing
   object for which the access rights are granted.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The two only interesting error codes are 

   * EFAULT: the filesystem is in a bad state, we can't do anything

   * ENOENT: the file that we are supposed to open doesn't exist (or a
             subpath).

   It is possible to restrict ourselves to only these two error
   conditions. It is also possible to implement more detailed error
   condition answers.

   The error codes are documented in man 2 open.

*/
int __myfs_open_implem(void *fsptr, size_t fssize, int *errnoptr,
                       const char *path) {
    /* Cast the file system data structure pointer to the correct type */
    struct myfs_data *fsdata = (struct myfs_data *)fsptr;

    /* Look up the file in the root directory */
    for (int i = 0; i < fsdata->num_files; i++) {
        if (strcmp(path, fsdata->root_dir[i].name) == 0) {
            /* Check if the current user has permission to open the file */
            if (fsdata->inode_table[fsdata->root_dir[i].inode].mode & O_RDWR) {
                return 0;
            } else {
                *errnoptr = EACCES;
                return -1;
            }
        }
    }

    /* Return an error if the file was not found */
    *errnoptr = ENOENT;
    return -1;
}

/* Implements an emulation of the read system call on the filesystem 
   of size fssize pointed to by fsptr.

   The call copies up to size bytes from the file indicated by 
   path into the buffer, starting to read at offset. See the man page
   for read for the details when offset is beyond the end of the file etc.
   
   On success, the appropriate number of bytes read into the buffer is
   returned. The value zero is returned on an end-of-file condition.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 read.

*/
int __myfs_read_implem(void *fsptr, size_t fssize, int *errnoptr,
                       const char *path, char *buf, size_t size, off_t offset) {
    /* Cast the file system data structure pointer to the correct type */
    struct myfs_data *fsdata = (struct myfs_data *)fsptr;

    /* Look up the file in the root directory */
    for (int i = 0; i < fsdata->num_files; i++) {
        if (strcmp(path, fsdata->root_dir[i].name) == 0) {
            /* Check if the offset is within the file's size */
            if (offset < fsdata->inode_table[fsdata->root_dir[i].inode].size) {
                /* Calculate the data block and offset within the block to start reading from */
                int block_num = offset / BLOCK_SIZE;
                int block_offset = offset % BLOCK_SIZE;

                /* Copy data from the file's data blocks into the provided buffer */
                size_t bytes_read = 0;
                while (bytes_read < size && offset < fsdata->inode_table[fsdata->root_dir[i].inode].size) {
                    buf[bytes_read] = fsdata->data_block_table[fsdata->inode_table[fsdata->root_dir[i].inode].data_block[block_num]].data[block_offset];
                    bytes_read++;
                    block_offset++;

                    if (block_offset >= BLOCK_SIZE) {
                        block_num++;
                        block_offset = 0;
                    }
                }

                return bytes_read;
            } else {
                /* Return 0 if the offset is beyond the end of the file */
                return 0;
            }
        }
    }

    /* Return an error if the file was not found */
    *errnoptr = ENOENT;
    return -1;
}

/* Implements an emulation of the write system call on the filesystem 
   of size fssize pointed to by fsptr.

   The call copies up to size bytes to the file indicated by 
   path into the buffer, starting to write at offset. See the man page
   for write for the details when offset is beyond the end of the file etc.
   
   On success, the appropriate number of bytes written into the file is
   returned. The value zero is returned on an end-of-file condition.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 write.

*/
int __myfs_write_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path, const char *buf, size_t size, off_t offset) {
  /* Cast the file system data structure pointer to the correct type */
    struct myfs_data *fsdata = (struct myfs_data *)fsptr;

    /* Look up the file in the root directory */
    for (int i = 0; i < fsdata->num_files; i++) {
        if (strcmp(path, fsdata->root_dir[i].name) == 0) {
            /* Allocate additional data blocks if necessary to store the new data */
            int num_blocks = (offset + size) / BLOCK_SIZE;
            if ((offset + size) % BLOCK_SIZE != 0) {
                num_blocks++;
            }

            if (num_blocks > fsdata->inode_table[fsdata->root_dir[i].inode].data_block) {
                for (int j = fsdata->inode_table[fsdata->root_dir[i].inode].data_block; j < num_blocks; j++) {
                    fsdata->data_block_table[j].used = 1;
                }
            }

            /* Calculate the data block and offset within the block to start writing at */
            int block_num = offset / BLOCK_SIZE;
            int block_offset = offset % BLOCK_SIZE;

            /* Copy data from the provided buffer into the file's data blocks */
            size_t bytes_written = 0;
            while (bytes_written < size) {
                fsdata->data_block_table[fsdata->inode_table[fsdata->root_dir[i].inode].data_block[block_num]].data[block_offset] = buf[bytes_written];
                bytes_written++;
                block_offset++;

                if (block_offset >= BLOCK_SIZE) {
                    block_num++;
                    block_offset = 0;
                }
            }

            /* Update the file size */
            if (offset + bytes_written > fsdata->inode_table[fsdata->root_dir[i].inode].size) {
                fsdata->inode_table[fsdata->root_dir[i].inode].size = offset + bytes_written;
            }

            return bytes_written;
        }
    }

    /* Return an error if the file was not found */
    *errnoptr = ENOENT;
    
    return -1;
}

/* Implements an emulation of the utimensat system call on the filesystem 
   of size fssize pointed to by fsptr.

   The call changes the access and modification times of the file
   or directory indicated by path to the values in ts.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 utimensat.

*/
int __myfs_utimens_implem(void *fsptr, size_t fssize, int *errnoptr,
                          const char *path, const struct timespec ts[2]) {
  /* Cast the file system data structure pointer to the correct type */
    struct myfs_data *fsdata = (struct myfs_data *)fsptr;

    /* Look up the file or directory in the root directory */
    for (int i = 0; i < fsdata->num_files; i++) {
        if (strcmp(path, fsdata->root_dir[i].name) == 0) {
            /* Update the access and modification times in the inode table */
            fsdata->inode_table[fsdata->root_dir[i].inode].access_time = ts[0];
            fsdata->inode_table[fsdata->root_dir[i].inode].modification_time = ts[1];

            return 0;
        }
    }

    /* Return an error if the file or directory was not found */
    *errnoptr = ENOENT;
    return -1;
}

/* Implements an emulation of the statfs system call on the filesystem 
   of size fssize pointed to by fsptr.

   The call gets information of the filesystem usage and puts in 
   into stbuf.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 statfs.

   Essentially, only the following fields of struct statvfs need to be
   supported:

   f_bsize   fill with what you call a block (typically 1024 bytes)
   f_blocks  fill with the total number of blocks in the filesystem
   f_bfree   fill with the free number of blocks in the filesystem
   f_bavail  fill with same value as f_bfree
   f_namemax fill with your maximum file/directory name, if your
             filesystem has such a maximum

*/
int __myfs_statfs_implem(void *fsptr, size_t fssize, int *errnoptr,
                         struct statvfs* stbuf) {
 /* Cast the file system data structure pointer to the correct type */
    struct myfs_data *fsdata = (struct myfs_data *)fsptr;

    /* Calculate the total size of the file system */
    stbuf->f_blocks = fsdata->num_data_blocks * BLOCK_SIZE;

    /* Calculate the number of free data blocks in the file system */
    stbuf->f_bfree = 0;
    for (int i = 0; i < fsdata->num_data_blocks; i++) {
        if (!fsdata->data_block_table[i].used) {
            stbuf->f_bfree++;
        }
    }

    /* Calculate the number of inodes in the file system */
    stbuf->f_files = fsdata->num_inodes;

    /* Calculate the number of free inodes in the file system */
    stbuf->f_ffree = 0;
    for (int i = 0; i < fsdata->num_inodes; i++) {
        if (!fsdata->inode_table[i].used) {
            stbuf->f_ffree++;
        }
    }

    return 0;
}
