/*
James McAllister 
1001078112 

Randy Bui
1001338008
*/


#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)


static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;
static int maximum_heap      = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};


struct _block *freeList = NULL; /* Free list to track the _blocks available */
struct _block *nextFit = NULL;  //The struct that holds our position during nextFit
/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = freeList;

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
   //Initialize a struct to keep track of our max location
   struct _block *maxCurr = NULL;
   size_t min = INT_MAX;
   while (curr!=NULL)
   {  
      //If the current block is free and the difference in size is smaller than the previous difference
      //Then we set our new best fit block to this current location and updated our variable
      if(curr->free && curr->size >= size && (curr->size-size)<min)
      {
         maxCurr = curr;
         min = curr->size - size;
      }
      
      *last = curr;
      curr = curr->next;
   }
   curr = maxCurr;
#endif

#if defined WORST && WORST == 0
   //Initialize a block struct to keep track of our max location
   struct _block *maxCurr = NULL;
   int min = -1;
   while (curr!=NULL)
   {  
      if(curr->free && curr->size >= size)
      {
         //if the minimum is still -1 then we can set the first block to the worst fit
         if(min == -1)
         {
            maxCurr = curr;
            min = curr->size - size;
         }
         //Else we will check to see if the difference is greater than the previous difference
         else if(min<(curr->size-size))
         {
         maxCurr = curr;
         min = curr->size - size;
         }
      }
      
      *last = curr;
      curr = curr->next;
   }
   //set curr to the max value found
   curr = maxCurr;
#endif

#if defined NEXT && NEXT == 0
   int flagLoop = 0;
   if(nextFit!=NULL)
   {
      curr = nextFit; 
   }
      while (curr && !(curr->free && (curr->size >= size))) 
      {
         if(curr->next == NULL)
         {
            //If we have looped one time already break out and return NULL block
            if(flagLoop == 1)
            {
               *last = curr;
               curr = NULL;
               break;
            }
            flagLoop++;
            *last = curr;
            curr = freeList;
         }else
         {
            //updates
            *last = curr;
             curr  = curr->next;
         }
      } 
      //updated our current location in the freelist
      nextFit = curr; 
#endif
   //Split the block if the size requested is smaller than the size of the block found
   //If block given is null then ignore
   if(curr!=NULL)
   {
      if(size<curr->size)
      {
      
      size_t currSize = curr->size;
      struct _block *currNext = curr->next;
      void * ptr = BLOCK_DATA(curr) + size/24;
      //Set currs size to the size requested
      curr->size = size;
      //set the curr's next to our newly calculated pointer location
      curr->next = BLOCK_HEADER(ptr);
      //Set the newly created block to free
      curr->next->free = true;
      //updating the size of our newly split block accounting for the size of the data
      curr->next->size = currSize - size - sizeof(struct _block);
      //This sets the newly split block next pointer to what was the original next block
      curr->next->next = currNext;
      //If we split then the number of splits will increment and the number of blocks also
      num_splits++;
      num_blocks++;

      }
   }
   //returns either a free block or a null block to the system
   return curr;
}
/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update freeList if not set */
   if (freeList == NULL) 
   {
      freeList = curr;
   }

   /* Attach new _block to prev _block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update _block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   num_grows++;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{
   maximum_heap += size;
   if(maximum_heap > max_heap)
   {
      max_heap = maximum_heap;
   }
   num_requested += size;

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block */
   struct _block *last = freeList;
   struct _block *next = findFreeBlock(&last, size);

   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
      num_blocks++;
   }
   else
   {
      num_reuses++;
   }
   

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   
   /* Mark _block as in use */
   next->free = false;

   /* Return data address associated with _block */
   num_mallocs++;
   return BLOCK_DATA(next);
}

void* calloc(size_t nmemb, size_t size)
{
  void *ptr;
  size_t total_size = nmemb*size; 
  if(nmemb == 0 || size == 0)
  {
     return NULL;
  }
  else
  {
     ptr = malloc(total_size);
     bzero(ptr,total_size);
     return ptr;
  }

}


void *realloc(void *ptr,size_t size)
{
   
   void * ptr2 = malloc(size);
   struct _block *curr = BLOCK_HEADER(ptr);
   memcpy(ptr2,ptr,curr->size);
   free(ptr);
   return ptr2;
}


/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{

   if (ptr == NULL) 
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   maximum_heap -= curr->size;
   assert(curr->free == 0);
   curr->free = true;

   //Colesce 
   //Set a pointer to the beggingin of our list
   //Iterate through the entire list while the current block is not null
   //If we find a block that is free and its next block is also free 
   //Then we can combine the two and update the list appropriately
   struct _block *beginning = freeList;
   while(beginning)
   {
      if(beginning->free && beginning->next != NULL && beginning->next->free)
      {
         beginning->size = beginning->size + beginning->next->size;
         if(beginning->next->next == NULL)
         {
            beginning->next = NULL;
         }
         else
         {
            beginning->next = beginning->next->next;
         }
         beginning->free = true;
         num_blocks--;
         num_coalesces++;
      }
      beginning = beginning->next;
   }
   num_frees++;
   
}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
