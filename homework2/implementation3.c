/*  

    Copyright 2018-21 by

    University of Alaska Anchorage, College of Engineering.

    Copyright 2022 by

    University of Texas at El Paso, Department of Computer Science.

    All rights reserved.

    Contributors:  ...
                   ...
		   ...                 and
		   Christoph Lauter

    See file memory.c on how to compile this code.

    Implement the functions __malloc_impl, __calloc_impl,
    __realloc_impl and __free_impl below. The functions must behave
    like malloc, calloc, realloc and free but your implementation must
    of course not be based on malloc, calloc, realloc and free.

    Use the mmap and munmap system calls to create private anonymous
    memory mappings and hence to get basic access to memory, as the
    kernel provides it. Implement your memory management functions
    based on that "raw" access to user space memory.

    As the mmap and munmap system calls are slow, you have to find a
    way to reduce the number of system calls, by "grouping" them into
    larger blocks of memory accesses. As a matter of course, this needs
    to be done sensibly, i.e. without wasting too much memory.

    You must not use any functions provided by the system besides mmap
    and munmap. If you need memset and memcpy, use the naive
    implementations below. If they are too slow for your purpose,
    rewrite them in order to improve them!

    Catch all errors that may occur for mmap and munmap. In these cases
    make malloc/calloc/realloc/free just fail. Do not print out any 
    debug messages as this might get you into an infinite recursion!

    Your __calloc_impl will probably just call your __malloc_impl, check
    if that allocation worked and then set the fresh allocated memory
    to all zeros. Be aware that calloc comes with two size_t arguments
    and that malloc has only one. The classical multiplication of the two
    size_t arguments of calloc is wrong! Read this to convince yourself:

    https://bugzilla.redhat.com/show_bug.cgi?id=853906

    In order to allow you to properly refuse to perform the calloc instead
    of allocating too little memory, the __try_size_t_multiply function is
    provided below for your convenience.
    
*/

#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>


#define MIN_MAP_SIZE 16777216
#define ALIGNMENT 16
#define HEADSIZE (sizeof(free_blk_header_t)+sizeof(block_footer))
#define CHUNK_SIZE (1 << 14)
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define CHUNK_ALIGN(size) (((size)+(CHUNK_SIZE-1)) & ~(CHUNK_SIZE-1))
#define GET_HEADER(bp) ((char *)(bp) - HEADSIZE)
#define GET_SIZE(p) ((free_blk_header_t *)(p))->size
#define GET_ALLOC(p) ((free_blk_header_t *)(p))->allocated
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(GET_HEADER(bp)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE((char *)(bp)-HEADSIZE))
#define FTRP(bp) ((char *)(bp)+GET_SIZE(GET_HEADER(bp))-HEADSIZE)
#define NEXT_BLKP2(sausage)  ((free_blk_header_t *)(sausage))->next
#define PREV_BLKP2(sausage) ((free_blk_header_t *)(sausage))->prev

/* Predefined helper functions */

static void *__memset(void *s, int c, size_t n) {
  unsigned char *p;
  size_t i;

  if (n == ((size_t) 0)) return s;
  for (i=(size_t) 0,p=(unsigned char *)s;
       i<=(n-((size_t) 1));
       i++,p++) {
    *p = (unsigned char) c;
  }
  return s;
}

static void *__memcpy(void *dest, const void *src, size_t n) {
  unsigned char *pd;
  const unsigned char *ps;
  size_t i;

  if (n == ((size_t) 0)) return dest;
  for (i=(size_t) 0,pd=(unsigned char *)dest,ps=(const unsigned char *)src;
       i<=(n-((size_t) 1));
       i++,pd++,ps++) {
    *pd = *ps;
  }
  return dest;
}

/* Tries to multiply the two size_t arguments a and b.

   If the product holds on a size_t variable, sets the 
   variable pointed to by c to that product and returns a 
   non-zero value.
   
   Otherwise, does not touch the variable pointed to by c and 
   returns zero.

   This implementation is kind of naive as it uses a division.
   If performance is an issue, try to speed it up by avoiding 
   the division while making sure that it still does the right 
   thing (which is hard to prove).

*/
static int __try_size_t_multiply(size_t *c, size_t a, size_t b) {
  size_t t, r, q;

  /* If any of the arguments a and b is zero, everthing works just fine. */
  if ((a == ((size_t) 0)) ||
      (b == ((size_t) 0))) {
    *c = a * b;
    return 1;
  }

  /* Here, neither a nor b is zero. 

     We perform the multiplication, which may overflow, i.e. present
     some modulo-behavior.

  */
  t = a * b;

  /* Perform Euclidian division on t by a:

     t = a * q + r

     As we are sure that a is non-zero, we are sure
     that we will not divide by zero.

  */
  q = t / a;
  r = t % a;

  /* If the rest r is non-zero, the multiplication overflowed. */
  if (r != ((size_t) 0)) return 0;

  /* Here the rest r is zero, so we are sure that t = a * q.

     If q is different from b, the multiplication overflowed.
     Otherwise we are sure that t = a * b.

  */
  if (q != b) return 0;
  *c = t;
  return 1;
}

/* End of predefined helper functions */

/* Your helper functions 

   You may also put some struct definitions, typedefs and global
   variables here. Typically, the instructor's solution starts with
   defining a certain struct, a typedef and a global variable holding
   the start of a linked list of currently free memory blocks. That 
   list probably needs to be kept ordered by ascending addresses.

*/
typedef struct free_blk_header {
  size_t size;
  struct free_blk_header *next;
  char allocated;
  struct free_blk_header *prev;
} free_blk_header_t;

typedef struct {
  size_t size;
  int filler;
} block_footer;

void *free_mem_blocks = NULL;
void *first_sausage;
size_t mem_blocks_size = 0;
int mminit = 0;

//initializing linked list
void mm_init() {
  sbrk(sizeof(free_blk_header_t));
  first_sausage = sbrk(0);
  //add prolog block?
  //__malloc_impl((size_t) 0);
  GET_SIZE(GET_HEADER(first_sausage)) = 0;
  GET_ALLOC(GET_HEADER(first_sausage)) = 1;
  return;
}


//coalesce fragmented sausages
void *coalesce(void *sausage){
  //determine whether adjacent sausages are allocated
  size_t prev_alloc = GET_ALLOC(GET_HEADER(PREV_BLKP(sausage)));
  size_t next_alloc = GET_ALLOC(GET_HEADER(NEXT_BLKP(sausage)));
  size_t size = GET_SIZE(GET_HEADER(sausage));

  if(prev_alloc && next_alloc){//if both used, do nothing
  }
  else if(prev_alloc && !next_alloc){
    //merge with next block
    size += GET_SIZE(GET_HEADER(NEXT_BLKP(sausage)));
    GET_SIZE(GET_HEADER(sausage)) = size; //change size
    GET_SIZE(FTRP(sausage)) = size;
  }
  else if(!prev_alloc && next_alloc){
    //merge with previous block
    size += GET_SIZE(GET_HEADER(PREV_BLKP(sausage)));
    GET_SIZE(FTRP(sausage)) = size;
    GET_SIZE(GET_HEADER(PREV_BLKP(sausage))) = size;
    sausage = PREV_BLKP(sausage);
  }
  else{
    //merge previous and next block
    size +=  GET_SIZE(GET_HEADER(NEXT_BLKP(sausage))) + GET_SIZE(GET_HEADER(PREV_BLKP(sausage)));
    GET_SIZE(GET_HEADER(PREV_BLKP(sausage))) = size;
    GET_SIZE(FTRP(NEXT_BLKP(sausage))) = size;
    sausage = PREV_BLKP(sausage);
  }
  return sausage;
}

//EXTEND
void get_block(size_t rawsize){
  size_t chunk_size = CHUNK_ALIGN(rawsize); //align chunk
  void *sausage = sbrk(chunk_size); //increase heap by chunk size
  GET_SIZE(GET_HEADER(sausage)) = chunk_size;
  GET_SIZE(FTRP(sausage)) = chunk_size;
  GET_ALLOC(GET_HEADER(sausage)) = 0;
  /*need to point it to thw start of the list
  NEXT_BLKP(sausage) = first_sausage;
  PREV_BLKP(first_sausage) = sausage;
  first_sausage = sausage;*/
  
  GET_SIZE(GET_HEADER(NEXT_BLKP(sausage))) = 0;
  GET_ALLOC(GET_HEADER(NEXT_BLKP(sausage))) = 1;
}

//set_allocated
void new_block(void *sausage, size_t size){
  size_t extra_size = GET_SIZE(GET_HEADER(sausage)) - size;
  if (extra_size > ALIGN(1 + HEADSIZE)) {
    GET_SIZE(GET_HEADER(sausage)) = size;
    GET_SIZE(FTRP(sausage)) = size;
    GET_SIZE(GET_HEADER(NEXT_BLKP(sausage))) = extra_size;
    GET_SIZE(FTRP(NEXT_BLKP(sausage))) = extra_size;
    GET_ALLOC(GET_HEADER(NEXT_BLKP(sausage))) = 0;
  }
  GET_ALLOC(GET_HEADER(sausage)) = 1;
}

/* End of your helper functions */

/* Start of the actual malloc/calloc/realloc/free functions */

void __free_impl(void *);

void *__malloc_impl(size_t size) {
  /*
    initially create a block (mmap) of 'size'
    search through recycle bin for something that is close to size
    and use that.
 */
  int new_size = ALIGN(size + HEADSIZE);
  void *sausage = first_sausage; //start at head
  
  while (GET_SIZE(GET_HEADER(sausage)) != 0) {
    //search through sausages until you find one thats the right size and unallocated
    if (!GET_ALLOC(GET_HEADER(sausage))
	&& (GET_SIZE(GET_HEADER(sausage)) >= new_size)) {
      new_block(sausage, new_size);
      return sausage;
    }
    sausage = NEXT_BLKP(sausage); //go to next sausage
  }
  //couldnt find one, so create a new sausage
  get_block(new_size); //extend list
  new_block(sausage, new_size); //set new sausage to allocated
  return sausage;
}

//start with this one
void *__calloc_impl(size_t nmemb, size_t size) {
  /* if either argument is 0 return NULL
   multiply args and give that result to malloc
   if
   */
  size_t zero = 0, s;
  void *ptr;

  //
  if(__try_size_t_multiply(&s, nmemb, size)){return NULL;}
  ptr = (void *) __malloc_impl(s);
  if(ptr != NULL){
    __memset(ptr, zero, s);
  }
  
  return ptr;  
}

//then this one
void *__realloc_impl(void *ptr, size_t size) {
  /*
    if ptr = NULL return malloc size, if size 
    = 0 return free(ptr), malloc what is asked
    copy the old data to the new place, free the
    address.
 */
  void *newptr, *old_sausage;
  size_t s;
  
  if(ptr == NULL){return __malloc_impl(size);}
  if(size == 0){
    __free_impl(ptr);
    return NULL;
  }
  newptr = __malloc_impl(size);
  if(newptr == NULL){return NULL;}
  old_sausage = ptr - HEADSIZE;
  //might replace sausage with ptr
  s = GET_SIZE(GET_HEADER(old_sausage));
  if(size > s){s = size;}
  __memcpy(newptr, ptr, s);
  __free_impl(ptr);
  return newptr;
}

//3rd
void __free_impl(void *sausage) {
  /*
    if ptr is NULL return,
    find size of ptr by putting header in front of ptr
    which is a pointer to another header or null. make
    recycle bin to store free'd memory.
    make a struct for the header type with attr's 
    size_t size_of_block, struct _mem_block_Struct_t *next
    void *mmap_ptr, size_t mmap_size. and a typedef struct
    mem_block_struct_t mem_block_t. 
    
 */
  if(sausage==NULL){return;}
  GET_ALLOC(GET_HEADER(sausage)) = 0;//mark unallocated
  coalesce(sausage);//coalesce if needed
}

/* End of the actual malloc/calloc/realloc/free functions */

