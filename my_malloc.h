#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
//#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

typedef struct metaData {
    size_t size;
    struct metaData * prev;
    struct metaData * next;
    void * afterMeta;
} meta;

#define metaSize sizeof(meta)
meta * freeList_headPtr = NULL;
meta * freeList_tailPtr = NULL;
unsigned long data_segment_size = 0;
unsigned long data_segment_free_space_size = 0;
void *ff_malloc(size_t size);
void *bf_malloc(size_t size);
void ff_free(void *ptr);
void bf_free(void *ptr);
void *request_newBlock(size_t size);
void addBackToFreeList(meta * newMeta_ptr);
void *split_oldBlock(meta * curPtr, size_t size);
void deleteFromFreeList(meta * curPtr);
void mergeBlock(meta * newMeta_ptr);
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();

